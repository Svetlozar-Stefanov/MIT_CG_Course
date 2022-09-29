#ifndef INTEGRATOR_H
#define INTEGRATOR_H

#include "vecmath.h"
#include <vector>
#include "particlesystem.h"

class TimeStepper
{
public:
    virtual ~TimeStepper() {}
	virtual void takeStep(ParticleSystem* particleSystem, float stepSize) = 0;
};

//IMPLEMENT YOUR TIMESTEPPERS

class ForwardEuler : public TimeStepper
{
	void takeStep(ParticleSystem* particleSystem, float stepSize) override;
};

class Trapezoidal : public TimeStepper
{
	void takeStep(ParticleSystem* particleSystem, float stepSize) override;
	std::vector<Vector3f> evalFOStep(ParticleSystem* particleSystem, float stepSize);
};

class RK4 : public TimeStepper
{
	std::vector<Vector3f> scale(const std::vector<Vector3f> &state, float scale, const std::vector<Vector3f> &f);
	void takeStep(ParticleSystem* particleSystem, float stepSize) override;
};

/////////////////////////
#endif
