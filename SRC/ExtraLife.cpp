#include "ExtraLife.h"

ExtraLife::ExtraLife() 
{
	mType = GameObjectType("ExtraLife");
	powerUp_shape = make_shared<Shape>("extraLife.shape");
}

ExtraLife::~ExtraLife() {}