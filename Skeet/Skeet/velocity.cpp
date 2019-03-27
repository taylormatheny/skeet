#include "velocity.h"

Velocity::Velocity()
{
	dx = 0;
	dy = 0;
}

Velocity::Velocity(float dx, float dy)
{
	dx = getDx();
	dy = getDy();
}

float Velocity::getDx()
{
	return dx;
}

float Velocity::getDy()
{
	return dy;
}

void Velocity::setDx(float dx)
{
	this->dx = dx;
}

void Velocity::setDy(float dy)
{
	this->dy = dy;
}
