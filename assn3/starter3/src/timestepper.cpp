#include "timestepper.h"

#include <cstdio>

void ForwardEuler::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1
	std::vector<Vector3f> currentState = particleSystem->getState();
	std::vector<Vector3f> f = particleSystem->evalF(currentState);
	
	std::vector<Vector3f> newState;

	for (int i = 0; i < currentState.size(); i++)
	{
		newState.push_back(currentState[i] + stepSize * f[i]);
	}

	particleSystem->setState(newState);
}

void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1 
}


void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
}

