#ifndef __ASTEROIDS_H__
#define __ASTEROIDS_H__

#include "GameUtil.h"
#include "GameSession.h"
#include "IKeyboardListener.h"
#include "IGameWorldListener.h"
#include "IScoreListener.h" 
#include "ScoreKeeper.h"
#include "Player.h"
#include "IPlayerListener.h"
#include <vector>
#include <algorithm>

class GameObject;
class Spaceship;
class GUILabel;

class Asteroids : public GameSession, public IKeyboardListener, public IGameWorldListener, public IScoreListener, public IPlayerListener
{
public:
	Asteroids(int argc, char* argv[]);
	virtual ~Asteroids(void);

	virtual void Start(void);
	virtual void Stop(void);

	// Declaration of IKeyboardListener interface ////////////////////////////////

	void OnKeyPressed(uchar key, int x, int y);
	void OnKeyReleased(uchar key, int x, int y);
	void OnSpecialKeyPressed(int key, int x, int y);
	void OnSpecialKeyReleased(int key, int x, int y);

	// Declaration of IScoreListener interface //////////////////////////////////

	void OnScoreChanged(int score);

	// Declaration of the IPlayerLister interface //////////////////////////////

	void OnPlayerKilled(int lives_left);

	// Declaration of IGameWorldListener interface //////////////////////////////

	void OnWorldUpdated(GameWorld* world) {}
	void OnObjectAdded(GameWorld* world, shared_ptr<GameObject> object) {}
	void OnObjectRemoved(GameWorld* world, shared_ptr<GameObject> object);

	// Override the default implementation of ITimerListener ////////////////////
	void OnTimer(int value);

	// Scores implementation
	void LoadScores();
	void SaveScore(const std::string& name, int score);

	// Powerups implementation
	void SpawnPowerUp();

	// Boost implementation
	void UpdateFuelText();

private:
	shared_ptr<Spaceship> mSpaceship;
	shared_ptr<GUILabel> mScoreLabel;
	shared_ptr<GUILabel> mLivesLabel;
	shared_ptr<GUILabel> mBoostLabel;
	shared_ptr<GUILabel> mGameOverLabel;

	shared_ptr<GUILabel> mStartLabel;
	shared_ptr<GUILabel> mDifficultyLabel;
	shared_ptr<GUILabel> mInstructionsTitleLabel;
	shared_ptr<GUILabel> mInstructionsLabel1;
	shared_ptr<GUILabel> mInstructionsLabel2;
	shared_ptr<GUILabel> mInstructionsLabel3;
	shared_ptr<GUILabel> mHighscoresTitleLabel;
	std::vector<shared_ptr<GUILabel>> mDisplayedScores;

	uint mLevel;
	uint mAsteroidCount;

	void StartGame();

	void CreateMenu();
	void HideMenu();

	// Difficulty selector
	void SetDifficulty(std::string difficulty);

	void ResetSpaceship();
	shared_ptr<GameObject> CreateSpaceship();
	void CreateGUI();
	void CreateAsteroids(const uint num_asteroids);
	shared_ptr<GameObject> CreateExplosion();

	const static uint SHOW_GAME_OVER = 0;
	const static uint START_NEXT_LEVEL = 1;
	const static uint CREATE_NEW_PLAYER = 2;
	const static uint INVULNERABLE = 3;

	ScoreKeeper mScoreKeeper;
	Player mPlayer;

	enum GameState { MENU, PLAYING };
	GameState mState;

	enum Difficulty { NORMAL, HARD };
	Difficulty mDifficulty;

	struct ScoreEntry
	{
		std::string name;
		int score;
	};
	shared_ptr<GUILabel> mEnterNameLabel;
	shared_ptr<GUILabel> mNameInputLabel;
	std::string mNameInput;
	bool mEnteringName = false;
	std::vector<ScoreEntry> mScores;
};
#endif