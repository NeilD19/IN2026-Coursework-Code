#include "Asteroid.h"
#include "Asteroids.h"
#include "Animation.h"
#include "AnimationManager.h"
#include "GameUtil.h"
#include "GameWindow.h"
#include "GameWorld.h"
#include "GameDisplay.h"
#include "Spaceship.h"
#include "BoundingShape.h"
#include "BoundingSphere.h"
#include "GUILabel.h"
#include "Explosion.h"
#include "PowerUp.h"
#include "ExtraLife.h"
#include "Invulnerability.h"
#include "Fuel.h"

// PUBLIC INSTANCE CONSTRUCTORS ///////////////////////////////////////////////

/** Constructor. Takes arguments from command line, just in case. */
Asteroids::Asteroids(int argc, char *argv[])
	: GameSession(argc, argv)
{
	mLevel = 0;
	mAsteroidCount = 0;
	mPowerupCount = 0;
	mState = MENU;
}

/** Destructor. */
Asteroids::~Asteroids(void)
{
}

// PUBLIC INSTANCE METHODS ////////////////////////////////////////////////////

/** Start an asteroids game. */
void Asteroids::Start()
{
	// Create a shared pointer for the Asteroids game object - DO NOT REMOVE
	shared_ptr<Asteroids> thisPtr = shared_ptr<Asteroids>(this);

	// Add this class as a listener of the game world
	mGameWorld->AddListener(thisPtr.get());

	// Add this as a listener to the world and the keyboard
	mGameWindow->AddKeyboardListener(thisPtr);

	// Add a score keeper to the game world
	mGameWorld->AddListener(&mScoreKeeper);

	// Add this class as a listener of the score keeper
	mScoreKeeper.AddListener(thisPtr);

	// Create an ambient light to show sprite textures
	GLfloat ambient_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GLfloat diffuse_light[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient_light);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse_light);
	glEnable(GL_LIGHT0);

	Animation *explosion_anim = AnimationManager::GetInstance().CreateAnimationFromFile("explosion", 64, 1024, 64, 64, "explosion_fs.png");
	Animation *asteroid1_anim = AnimationManager::GetInstance().CreateAnimationFromFile("asteroid1", 128, 8192, 128, 128, "asteroid1_fs.png");
	Animation *spaceship_anim = AnimationManager::GetInstance().CreateAnimationFromFile("spaceship", 128, 128, 128, 128, "spaceship_fs.png");
	Animation *extralife_anim = AnimationManager::GetInstance().CreateAnimationFromFile("extralife", 256, 256, 256, 256, "extralife.png");
	Animation *invulnerability_anim = AnimationManager::GetInstance().CreateAnimationFromFile("invulnerability", 256, 256, 256, 256, "invulnerability.png");
	Animation *fuel_anim = AnimationManager::GetInstance().CreateAnimationFromFile("fuel", 256, 256, 256, 256, "fuel.png");
	Animation *aura_anim = AnimationManager::GetInstance().CreateAnimationFromFile("aura", 64, 512, 64, 64, "aura.png");

	// Create some asteroids and add them to the world
	CreateAsteroids(10);

	CreateMenu();

	// Add a player (watcher) to the game world
	mGameWorld->AddListener(&mPlayer);

	// Add this class as a listener of the player
	mPlayer.AddListener(thisPtr);

	// Start the game
	GameSession::Start();
}

/** Start the game */
void Asteroids::StartGame()
{
	mState = PLAYING;

	HideMenu();

	mGameWorld->AddObject(CreateSpaceship());

	//Create the GUI
	CreateGUI();
}

/** Stop the current game. */
void Asteroids::Stop()
{
	// Stop the game
	GameSession::Stop();
}

// PUBLIC INSTANCE METHODS IMPLEMENTING IKeyboardListener /////////////////////

void Asteroids::OnKeyPressed(uchar key, int x, int y)
{
	if (mState == PLAYING)
	{
		if (mEnteringName)
		{
			if (key == 13) // Press enter to save score
			{
				if (!mNameInput.empty())
				{
					SaveScore(mNameInput, mScoreKeeper.GetScore());
					Stop();
				}
			}
			else if (key == 8) // Backspace to delete
			{
				if (!mNameInput.empty())
				{
					mNameInput.pop_back();
				}
			}
			else if (isalnum(key))
			{
				mNameInput += (char)key;
			}
			mNameInputLabel->SetText(mNameInput.empty() ? "_" : mNameInput);
			return;
		}
		else
		{
			switch (key)
			{
			case ' ':
				mSpaceship->Shoot(); break;
			case 'z':
				mSpaceship->Boost(10); 
				UpdateFuelText();
				break;
			default: break;
			}
		}
	}
	else if (mState == MENU)
	{
		switch (key)
		{
		case ' ':
			StartGame(); break;
		default: break;
		}
	}
}

void Asteroids::OnKeyReleased(uchar key, int x, int y) {}

void Asteroids::OnSpecialKeyPressed(int key, int x, int y)
{
	if (mState == PLAYING)
	{
		switch (key)
		{
			// If up arrow key is pressed start applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(10); break;
			// If left arrow key is pressed start rotating anti-clockwise
		case GLUT_KEY_LEFT: mSpaceship->Rotate(135); break;
			// If right arrow key is pressed start rotating clockwise
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(-135); break;
			// Default case - do nothing
		default: break;
		}
	}
	else if (mState == MENU)
	{
		switch (key)
		{
		// Up and Down to select power ups
		case GLUT_KEY_UP:
			mDifficultyMenuSelection = (mDifficultyMenuSelection - 1 + 3) % 3; // Wrap around
			UpdateDifficultyLabels();
			break;
		case GLUT_KEY_DOWN:
			mDifficultyMenuSelection = (mDifficultyMenuSelection + 1) % 3; // Wrap around
			UpdateDifficultyLabels();
			break;
		// Left and right to toggle selected power up on or off
		case GLUT_KEY_LEFT:
		case GLUT_KEY_RIGHT:
			if (mDifficultyMenuSelection == 0) mExtraLifeEnabled = !mExtraLifeEnabled;
			if (mDifficultyMenuSelection == 1) mInvulnerabilityEnabled = !mInvulnerabilityEnabled;
			if (mDifficultyMenuSelection == 2) mFuelEnabled = !mFuelEnabled;
			UpdateDifficultyLabels();
			break;
		default: break;
		}
	}
}

void Asteroids::OnSpecialKeyReleased(int key, int x, int y)
{
	if (mState == PLAYING)
	{
		switch (key)
		{
			// If up arrow key is released stop applying forward thrust
		case GLUT_KEY_UP: mSpaceship->Thrust(0); break;
			// If left arrow key is released stop rotating
		case GLUT_KEY_LEFT: mSpaceship->Rotate(0); break;
			// If right arrow key is released stop rotating
		case GLUT_KEY_RIGHT: mSpaceship->Rotate(0); break;
			// Default case - do nothing
		default: break;
		}
	}
	else if (mState == MENU)
	{
		
	}
}


// PUBLIC INSTANCE METHODS IMPLEMENTING IGameWorldListener ////////////////////

void Asteroids::OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object)
{
	if (object->GetType() == GameObjectType("Asteroid"))
	{
		shared_ptr<GameObject> explosion = CreateExplosion();
		explosion->SetPosition(object->GetPosition());
		explosion->SetRotation(object->GetRotation());
		mGameWorld->AddObject(explosion);
		mAsteroidCount--;
		if (mAsteroidCount <= 0) 
		{ 
			SetTimer(500, START_NEXT_LEVEL); 
		}
		SpawnPowerUp();
	}

	// Increment player lives when extra life power up is collected
	if (object->GetType() == GameObjectType("ExtraLife"))
	{
		mPowerupCount--;
		mPlayer.IncrementLives();
		std::ostringstream msg_stream;
		msg_stream << "Lives: " << mPlayer.GetLives();
		std::string lives_msg = msg_stream.str();
		mLivesLabel->SetText(lives_msg);
	}

	// 10 seconds of invulnerability
	if (object->GetType() == GameObjectType("Invulnerability"))
	{
		mPowerupCount--;
		mSpaceship->SetInvulnerability(true);
		SetTimer(10000, INVULNERABLE);
	}

	// Refuel
	if (object->GetType() == GameObjectType("Fuel"))
	{
		mPowerupCount--;
		mSpaceship->Refuel();
		UpdateFuelText();
	}
}

// PUBLIC INSTANCE METHODS IMPLEMENTING ITimerListener ////////////////////////

void Asteroids::OnTimer(int value)
{
	if (value == CREATE_NEW_PLAYER)
	{
		mSpaceship->Reset();
		// Temporary invulnerability after respawning
		mSpaceship->SetInvulnerability(true);
		SetTimer(1000, INVULNERABLE);
		mGameWorld->AddObject(mSpaceship);
	}

	if (value == START_NEXT_LEVEL)
	{
		mLevel++;
		int num_asteroids = 10 + 2 * mLevel;
		CreateAsteroids(num_asteroids);
		// Temporary invulnerability after starting next level
		mSpaceship->SetInvulnerability(true);
		SetTimer(1000, INVULNERABLE);
	}

	if (value == SHOW_GAME_OVER)
	{
		mGameOverLabel->SetVisible(true);

		// Initialise name input variables
		mEnteringName = true;
		mNameInput = "";

		// Show enter name label
		mEnterNameLabel = shared_ptr<GUILabel>(new GUILabel("Enter Name:"));
		mEnterNameLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_RIGHT);
		mEnterNameLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
		shared_ptr<GUIComponent> enterName_label_component = static_pointer_cast<GUIComponent>(mEnterNameLabel);
		mGameDisplay->GetContainer()->AddComponent(enterName_label_component, GLVector2f(0.5f, 0.4f));

		// Input display
		mNameInputLabel = shared_ptr<GUILabel>(new GUILabel("_"));
		mNameInputLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_LEFT);
		mNameInputLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
		shared_ptr<GUIComponent> nameInput_label_component = static_pointer_cast<GUIComponent>(mNameInputLabel);
		mGameDisplay->GetContainer()->AddComponent(nameInput_label_component, GLVector2f(0.51f, 0.4f));
	}

	if (value == INVULNERABLE)
	{
		mSpaceship->SetInvulnerability(false);
	}
}

// PROTECTED INSTANCE METHODS /////////////////////////////////////////////////
shared_ptr<GameObject> Asteroids::CreateSpaceship()
{
	// Create a raw pointer to a spaceship that can be converted to
	// shared_ptrs of different types because GameWorld implements IRefCount
	mSpaceship = make_shared<Spaceship>();
	mSpaceship->SetBoundingShape(make_shared<BoundingSphere>(mSpaceship->GetThisPtr(), 4.0f));
	shared_ptr<Shape> bullet_shape = make_shared<Shape>("bullet.shape");
	mSpaceship->SetBulletShape(bullet_shape);
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("spaceship");
	shared_ptr<Sprite> spaceship_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	mSpaceship->SetSprite(spaceship_sprite);
	mSpaceship->SetScale(0.1f);

	// Create invulnerability aura
	mAura = make_shared<GameObject>("Aura");
	Animation* aura_ptr = AnimationManager::GetInstance().GetAnimationByName("aura");
	shared_ptr<Sprite> aura_sprite = make_shared<Sprite>(aura_ptr->GetWidth(), aura_ptr->GetHeight(), aura_ptr);
	mAura->SetSprite(aura_sprite);
	mAura->SetScale(0); // Hide aura initially
	mGameWorld->AddObject(mAura);
	mSpaceship->SetAura(mAura);

	// Reset spaceship back to centre of the world
	mSpaceship->Reset();
	// Temporary invulnerability when spaceship spawns
	mSpaceship->SetInvulnerability(true);
	SetTimer(1000, INVULNERABLE);
	// Return the spaceship so it can be added to the world
	return mSpaceship;

}

void Asteroids::CreateAsteroids(const uint num_asteroids)
{
	mAsteroidCount = num_asteroids;
	for (uint i = 0; i < num_asteroids; i++)
	{
		Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("asteroid1");
		shared_ptr<Sprite> asteroid_sprite
			= make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
		asteroid_sprite->SetLoopAnimation(true);
		shared_ptr<GameObject> asteroid = make_shared<Asteroid>();
		asteroid->SetBoundingShape(make_shared<BoundingSphere>(asteroid->GetThisPtr(), 10.0f));
		asteroid->SetSprite(asteroid_sprite);
		asteroid->SetScale(0.2f);
		mGameWorld->AddObject(asteroid);
	}
}

void Asteroids::CreateGUI()
{
	// Add a (transparent) border around the edge of the game display
	mGameDisplay->GetContainer()->SetBorder(GLVector2i(10, 10));
	// Create a new GUILabel and wrap it up in a shared_ptr
	mScoreLabel = make_shared<GUILabel>("Score: 0");
	// Set the vertical alignment of the label to GUI_VALIGN_TOP
	mScoreLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> score_component
		= static_pointer_cast<GUIComponent>(mScoreLabel);
	mGameDisplay->GetContainer()->AddComponent(score_component, GLVector2f(0.0f, 1.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mLivesLabel = make_shared<GUILabel>("Lives: 3");
	// Set the vertical alignment of the label to GUI_VALIGN_BOTTOM
	mLivesLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_BOTTOM);
	// Add the GUILabel to the GUIComponent  
	shared_ptr<GUIComponent> lives_component = static_pointer_cast<GUIComponent>(mLivesLabel);
	mGameDisplay->GetContainer()->AddComponent(lives_component, GLVector2f(0.0f, 0.0f));

	// Create a new GUILabel and wrap it up in a shared_ptr
	mGameOverLabel = shared_ptr<GUILabel>(new GUILabel("GAME OVER"));
	// Set the horizontal alignment of the label to GUI_HALIGN_CENTER
	mGameOverLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	// Set the vertical alignment of the label to GUI_VALIGN_MIDDLE
	mGameOverLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	// Set the visibility of the label to false (hidden)
	mGameOverLabel->SetVisible(false);
	// Add the GUILabel to the GUIContainer  
	shared_ptr<GUIComponent> game_over_component
		= static_pointer_cast<GUIComponent>(mGameOverLabel);
	mGameDisplay->GetContainer()->AddComponent(game_over_component, GLVector2f(0.5f, 0.5f));

	// Boost label
	mBoostLabel = make_shared<GUILabel>("Boost: 100%");
	mBoostLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_RIGHT);
	mBoostLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	mBoostLabel->SetVisible(true);
	shared_ptr<GUIComponent> boost_component = static_pointer_cast<GUIComponent>(mBoostLabel);
	mGameDisplay->GetContainer()->AddComponent(boost_component, GLVector2f(1.0f, 1.0f));
}

void Asteroids::CreateMenu()
{
	// Create start label
	mStartLabel = shared_ptr<GUILabel>(new GUILabel("Press SPACE to Start"));
	mStartLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mStartLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_MIDDLE);
	shared_ptr<GUIComponent> start_label_component = static_pointer_cast<GUIComponent>(mStartLabel);
	mGameDisplay->GetContainer()->AddComponent(start_label_component, GLVector2f(0.5f, 0.4f));

	// Create difficulty label
	mDifficultyLabel = shared_ptr<GUILabel>(new GUILabel("Difficulty - Enable or Disable Power Ups:"));
	mDifficultyLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mDifficultyLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> difficulty_label_component = static_pointer_cast<GUIComponent>(mDifficultyLabel);
	mGameDisplay->GetContainer()->AddComponent(difficulty_label_component, GLVector2f(0.5f, 0.7f));
	// Create power-up toggle labels
	mExtraLifeLabel = shared_ptr<GUILabel>(new GUILabel("> Extra Life: On"));
	mExtraLifeLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mExtraLifeLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> extraLife_toggle_component = static_pointer_cast<GUIComponent>(mExtraLifeLabel);
	mGameDisplay->GetContainer()->AddComponent(extraLife_toggle_component, GLVector2f(0.5f, 0.65f));
	mInvulnerabilityLabel = shared_ptr<GUILabel>(new GUILabel("Invulnerability: On"));
	mInvulnerabilityLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mInvulnerabilityLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> invulnerability_toggle_component = static_pointer_cast<GUIComponent>(mInvulnerabilityLabel);
	mGameDisplay->GetContainer()->AddComponent(invulnerability_toggle_component, GLVector2f(0.5f, 0.6f));
	mFuelLabel = shared_ptr<GUILabel>(new GUILabel("Fuel: On"));
	mFuelLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mFuelLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> fuel_toggle_component = static_pointer_cast<GUIComponent>(mFuelLabel);
	mGameDisplay->GetContainer()->AddComponent(fuel_toggle_component, GLVector2f(0.5f, 0.55f));

	// Create instructions labels
	mInstructionsTitleLabel = shared_ptr<GUILabel>(new GUILabel("Instructions:"));
	mInstructionsTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> instructionsTitle_label_component = static_pointer_cast<GUIComponent>(mInstructionsTitleLabel);
	mGameDisplay->GetContainer()->AddComponent(instructionsTitle_label_component, GLVector2f(0.f, 1.0f));
	mInstructionsLabel1 = shared_ptr<GUILabel>(new GUILabel("1.Avoid the asteroids with the arrow keys"));
	mInstructionsLabel1->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> instructions_label1_component = static_pointer_cast<GUIComponent>(mInstructionsLabel1);
	mGameDisplay->GetContainer()->AddComponent(instructions_label1_component, GLVector2f(0.f, 0.95f));
	mInstructionsLabel2 = shared_ptr<GUILabel>(new GUILabel("2.Shoot with Space"));
	mInstructionsLabel2->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> instructions_label2_component = static_pointer_cast<GUIComponent>(mInstructionsLabel2);
	mGameDisplay->GetContainer()->AddComponent(instructions_label2_component, GLVector2f(0.f, 0.9f));
	mInstructionsLabel3 = shared_ptr<GUILabel>(new GUILabel("3.Boost with Z"));
	mInstructionsLabel3->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> instructions_label3_component = static_pointer_cast<GUIComponent>(mInstructionsLabel3);
	mGameDisplay->GetContainer()->AddComponent(instructions_label3_component, GLVector2f(0.f, 0.85f));

	LoadScores();
	mHighscoresTitleLabel = shared_ptr<GUILabel>(new GUILabel("High Scores:"));
	mHighscoresTitleLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
	mHighscoresTitleLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
	shared_ptr<GUIComponent> highscoresTitle_label_component = static_pointer_cast<GUIComponent>(mHighscoresTitleLabel);
	mGameDisplay->GetContainer()->AddComponent(highscoresTitle_label_component, GLVector2f(0.5f, 0.35f));

	// Display only the top 5 scores
	int maxToShow = min((int)mScores.size(), 5);
	mDisplayedScores.clear();
	for (int i = 0; i < maxToShow; i++)
	{
		std::string entry = std::to_string(i + 1) + ". " + mScores[i].name + ": " + std::to_string(mScores[i].score);
		shared_ptr<GUILabel> scoreEntryLabel = shared_ptr<GUILabel>(new GUILabel(entry));
		scoreEntryLabel->SetHorizontalAlignment(GUIComponent::GUI_HALIGN_CENTER);
		scoreEntryLabel->SetVerticalAlignment(GUIComponent::GUI_VALIGN_TOP);
		shared_ptr<GUIComponent> scoreEntry_label_component = static_pointer_cast<GUIComponent>(scoreEntryLabel);
		mGameDisplay->GetContainer()->AddComponent(scoreEntry_label_component, GLVector2f(0.5f, 0.3f - (i * 0.05f)));
		mDisplayedScores.push_back(scoreEntryLabel);
	}
}

void Asteroids::HideMenu()
{
	// Toggles all menu label visiblity off
	mStartLabel->SetVisible(false);
	mDifficultyLabel->SetVisible(false);
	mExtraLifeLabel->SetVisible(false);
	mInvulnerabilityLabel->SetVisible(false);
	mFuelLabel->SetVisible(false);
	mInstructionsTitleLabel->SetVisible(false);
	mInstructionsLabel1->SetVisible(false);
	mInstructionsLabel2->SetVisible(false);
	mInstructionsLabel3->SetVisible(false);
	mHighscoresTitleLabel->SetVisible(false);
	for (int i = 0; i < mDisplayedScores.size(); i++)
	{
		mDisplayedScores[i]->SetVisible(false);
	}
}

void Asteroids::OnScoreChanged(int score)
{
	// Format the score message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Score: " << score;
	// Get the score message as a string
	std::string score_msg = msg_stream.str();
	mScoreLabel->SetText(score_msg);
}

void Asteroids::OnPlayerKilled(int lives_left)
{
	shared_ptr<GameObject> explosion = CreateExplosion();
	explosion->SetPosition(mSpaceship->GetPosition());
	explosion->SetRotation(mSpaceship->GetRotation());
	mGameWorld->AddObject(explosion);

	// Format the lives left message using an string-based stream
	std::ostringstream msg_stream;
	msg_stream << "Lives: " << lives_left;
	// Get the lives left message as a string
	std::string lives_msg = msg_stream.str();
	mLivesLabel->SetText(lives_msg);

	if (lives_left > 0) 
	{ 
		SetTimer(1000, CREATE_NEW_PLAYER); 
	}
	else
	{
		SetTimer(500, SHOW_GAME_OVER);
	}
}

shared_ptr<GameObject> Asteroids::CreateExplosion()
{
	Animation *anim_ptr = AnimationManager::GetInstance().GetAnimationByName("explosion");
	shared_ptr<Sprite> explosion_sprite =
		make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	explosion_sprite->SetLoopAnimation(false);
	shared_ptr<GameObject> explosion = make_shared<Explosion>();
	explosion->SetSprite(explosion_sprite);
	explosion->Reset();
	return explosion;
}

void Asteroids::SaveScore(const std::string& name, int score)
{
	std::ofstream file("scores.txt", std::ios::app); // Open in append mode
	if (file.is_open())
	{
		file << name << " " << score << "\n"; // Append name and score
	}
}

void Asteroids::LoadScores()
{
	mScores.clear();
	std::ifstream file("scores.txt");
	if (!file.is_open()) return;

	std::string name;
	int score;
	while (file >> name >> score)
	{
		// Add every name and score pair in text file to list of scores
		mScores.push_back({ name, score });
	}

	// Sort in descending order
	std::sort(mScores.begin(), mScores.end(), [](const ScoreEntry& a, const ScoreEntry& b) { return a.score > b.score; });
}

void Asteroids::SpawnPowerUp()
{
	// 20% chance to spawn power up when asteroid destroyed
	// Only 3 power ups can exist at a time
	if (rand() % 5 != 0 || mPowerupCount == 3) return;

	shared_ptr<PowerUp> powerUp;
	std::string animation;
	int roll = rand() % 3; // Equal spawn chance for each power up
	if (roll == 0 && mExtraLifeEnabled)
	{
		powerUp = make_shared<ExtraLife>();
		animation = "extralife";
	}
	else if (roll == 1 && mInvulnerabilityEnabled)
	{
		powerUp = make_shared<Invulnerability>();
		animation = "invulnerability";
	}
	else if (roll == 2 && mFuelEnabled)
	{
		powerUp = make_shared<Fuel>();
		animation = "fuel";
	}
	else
	{
		// Prevent power up from being created if none enabled
		return;
	}

	powerUp->SetBoundingShape(make_shared<BoundingSphere>(powerUp->GetThisPtr(), 1.25f));
	//powerUp->SetShape(powerUp->GetShape());
	Animation* anim_ptr = AnimationManager::GetInstance().GetAnimationByName(animation);
	shared_ptr<Sprite> powerup_sprite = make_shared<Sprite>(anim_ptr->GetWidth(), anim_ptr->GetHeight(), anim_ptr);
	powerUp->SetSprite(powerup_sprite);
	powerUp->SetScale(0.025f);
	mGameWorld->AddObject(powerUp);
	mPowerupCount++;
}

void Asteroids::UpdateFuelText()
{
	int fuelPercent = (int)mSpaceship->GetFuel();
	std::ostringstream fuelStream;
	fuelStream << "Boost: " << fuelPercent << "%";
	mBoostLabel->SetText(fuelStream.str());
}

void Asteroids::UpdateDifficultyLabels()
{
	// Formats labels with a ">" if currently selected and shows if On or Off e.g "> Extra Life: On"
	std::string extraLifeText = std::string(mDifficultyMenuSelection == 0 ? "> " : "") + "Extra Life: " + (mExtraLifeEnabled ? "On" : "Off");
	mExtraLifeLabel->SetText(extraLifeText);
	std::string invulnerabilityText = std::string(mDifficultyMenuSelection == 1 ? "> " : "") + "Invulnerability: " + (mInvulnerabilityEnabled ? "On" : "Off");
	mInvulnerabilityLabel->SetText(invulnerabilityText);
	std::string fuelText = std::string(mDifficultyMenuSelection == 2 ? "> " : "") + "Fuel: " + (mFuelEnabled ? "On" : "Off");
	mFuelLabel->SetText(fuelText);
}