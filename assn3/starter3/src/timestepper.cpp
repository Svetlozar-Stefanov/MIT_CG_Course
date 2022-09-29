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

std::vector<Vector3f> RK4::scale(const std::vector<Vector3f>& state, float scale, const std::vector<Vector3f>& f)
{
	std::vector<Vector3f> newState;

	for (int i = 0; i < state.size(); i++)
	{
		newState.push_back(state[i] + scale * f[i]);
	}

	return newState;
}

void RK4::takeStep(ParticleSystem* particleSystem, float stepSize)
{
	std::vector<Vector3f> currentState = particleSystem->getState();

	std::vector<Vector3f> k1 = particleSystem->evalF(currentState);
	std::vector<Vector3f> k2 = particleSystem->evalF(scale(currentState, stepSize/2, k1));
	std::vector<Vector3f> k3 = particleSystem->evalF(scale(currentState, stepSize / 2, k2));
	std::vector<Vector3f> k4 = particleSystem->evalF(scale(currentState, stepSize, k3));

	for (int i = 0; i < currentState.size(); i++)
	{
		currentState[i] += (stepSize / 6) * (k1[i] + 2 * k2[i] + 2 * k3[i] + k4[i]);
	}

	particleSystem->setState(currentState);
}

