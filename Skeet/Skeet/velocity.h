/***********************************************************************
* Header File:
*
* Author:
*    Taylor Matheny
* Summary:
*
************************************************************************/

#ifndef VELOCITY_H
#define VELOCITY_H

class Velocity
{
private:
	// these cordinates determine the speed of the lander
	float dx;
	float dy;
public:
	// constructors
	Velocity();
	Velocity(float, float);

	// getters
	float getDx();
	float getDy();

	// setters
	void setDx(float dx);
	void setDy(float dy);
};
#endif
