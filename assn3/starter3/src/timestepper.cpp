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

std::vector<Vector3f> Trapezoidal::evalFOStep(ParticleSystem * particleSystem, float stepSize)
{
	std::vector<Vector3f> currentState = particleSystem->getState();
	std::vector<Vector3f> f = particleSystem->evalF(currentState);

	std::vector<Vector3f> newState;

	for (int i = 0; i < currentState.size(); i++)
	{
		newState.push_back(currentState[i] + stepSize * f[i]);
	}

	return newState;
}

void Trapezoidal::takeStep(ParticleSystem* particleSystem, float stepSize)
{
   //TODO: See handout 3.1
	std::vector<Vector3f> currentState = particleSystem->getState();
	std::vector<Vector3f> f0 = particleSystem->evalF(currentState);

	std::vector<Vector3f> f1;

	for (int i = 0; i < currentState.size(); i++)
	{
		f1.push_back(currentState[i] + stepSize * f0[i]);
	}

	f1 = particleSystem->evalF(f1);

	std::vector<Vector3f> newState;

	for (int i = 0; i < currentState.size(); i++)
	{
		newState.push_back(currentState[i] + (stepSize / 2) * (f0[i] + f1[i]));
	}

	particleSystem->setState(newState);
}


void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
}

