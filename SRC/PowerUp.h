#ifndef __POWERUP_H__
#define __POWERUP_H__

#include "GameObject.h"

class PowerUp : public GameObject
{
public:
	PowerUp();
	virtual ~PowerUp();

	bool CollisionTest(shared_ptr<GameObject> o);
	void OnCollision(const GameObjectList& objects);
};

#endif




