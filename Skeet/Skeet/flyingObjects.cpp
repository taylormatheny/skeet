#include "flyingObjects.h"

Point FlyingObjects::getPoint()
{
	return point;
}

Velocity FlyingObjects::getVelocity()
{
	return velocity;
}

bool FlyingObjects::isAlive()
{
	return alive;
}

void FlyingObjects::setPoint(Point point)
{
	this->point = point;
}

void FlyingObjects::setVelocity(Velocity velocity)
{
	this->velocity = velocity;
}

void FlyingObjects::kill()
{
	alive = false;
}

void FlyingObjects::advance()
{
	point.setX(point.getX() + velocity.getDx());
	point.setY(point.getY() + velocity.getDy());
}

