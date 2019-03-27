#pragma once
#ifndef FLYINGOBJECTS_H
#define FLYINGOBJECTS_H

#include <vector>

#include "uiDraw.h"
#include "uiInteract.h"
#include "point.h"
#include "velocity.h"
#include "rifle.h"

// TODO: include your bullet and bird classes
class FlyingObjects
{
private:
	Point point;
	Velocity velocity;
	bool alive;
public:
	Point getPoint();
	Velocity getVelocity();
	bool isAlive();
	void setPoint(Point point);
	void setVelocity(Velocity velocity);
	void kill();
	void advance();
};

#endif