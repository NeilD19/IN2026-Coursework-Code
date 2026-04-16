#include "PowerUp.h"
#include "GameUtil.h"
#include "BoundingShape.h"

PowerUp::PowerUp() : GameObject("PowerUp")
{
	mAngle = 0;
	mRotation = 30;
	mPosition.x = rand() / 2;
	mPosition.y = rand() / 2;
	mPosition.z = 0.0;
	mVelocity.x = 0.0;
	mVelocity.y = 0.0;
	mVelocity.z = 0.0;
}
PowerUp::~PowerUp() {}

bool PowerUp::CollisionTest(shared_ptr<GameObject> o)
{
	if (o->GetType() != GameObjectType("Spaceship")) return false;
	if (mBoundingShape.get() == NULL) return false;
	if (o->GetBoundingShape().get() == NULL) return false;
	return mBoundingShape->CollisionTest(o->GetBoundingShape());
}

void PowerUp::OnCollision(const GameObjectList& objects)
{
	mWorld->FlagForRemoval(GetThisPtr());
}