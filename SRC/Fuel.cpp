#include "Fuel.h"

Fuel::Fuel()
{
	mType = GameObjectType("Fuel");
	powerUp_shape = make_shared<Shape>("fuel.shape");
}

Fuel::~Fuel() {}
