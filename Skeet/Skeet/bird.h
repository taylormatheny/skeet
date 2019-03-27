#pragma once
#ifndef BIRD_H
#define BIRD_H

#include <vector>

#include "uiDraw.h"
#include "uiInteract.h"
#include "point.h"
#include "velocity.h"
#include "rifle.h"
#include "flyingObjects.h"
#include <cmath>

#define BULLET_SPEED 10.0
// TODO: include your bullet and bird classes
class Bird : public FlyingObjects
{
private:
	Point point;
	Velocity velocity;
	bool alive;
public:
	Bird();
	virtual int hit();
	Point getPoint();
	Velocity getVelocity();
	bool isAlive();
	void setAlive(bool alive);
	void setPoint(Point point);
	void setVelocity(Velocity velocity);
	void kill();
	virtual void draw() = 0;
	void advance();
};

class StandardBird : public Bird
{
private:
	Point point;
	Velocity velocity;
public:
	void draw();
};

class ToughBird : public Bird
{
private:
	int health;
	Velocity velocity;
	Point point;
public:
	ToughBird();
	void draw();
	void setHealth(int health);
	int hit();
};

class SacredBird : public Bird
{
public:
	void draw();
	int hit();
};
#endif
