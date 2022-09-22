#include "pendulumsystem.h"

#include <cassert>
#include "camera.h"
#include "vertexrecorder.h"

// TODO adjust to number of particles.
const int NUM_PARTICLES = 4;
const float G = 0.5f;
const float DRAG = 1.0f;

const int VEL_OFFSET = 1;

PendulumSystem::PendulumSystem()
{

	// TODO 4.2 Add particles for simple pendulum
	// TODO 4.3 Extend to multiple particles

	// To add a bit of randomness, use e.g.
	// float f = rand_uniform(-0.5f, 0.5f);
	// in your initial conditions.

	Particle end1;
	end1.pos = Vector3f(rand_uniform(-3, 3), 2, 0);
	end1.vel = Vector3f(0, 0, 0);
	end1.mass = 0;
	particles.push_back(end1);

	for (int i = 0; i < NUM_PARTICLES; i++)
	{
		Particle part;
		part.pos = Vector3f(rand_uniform((float)i, 3), 2, 0);
		part.vel = Vector3f(0, rand_uniform(-5, 5), 0);
		part.mass = rand_uniform(0.5, 1);
		particles.push_back(part);

		Spring sp;
		sp.restLen = 0.5f;
		sp.sturdiness = 2;
		sp.index = i;

		particles[i+1].springs.push_back(sp);
	}

	Particle end2;
	end2.pos = Vector3f(rand_uniform(-3, 3), 2, 0);
	end2.vel = Vector3f(0, 0, 0);
	end2.mass = 0;
	particles.push_back(end2);

	Spring sp;
	sp.restLen = 0.5f;
	sp.sturdiness = 2;
	sp.index = 5;

	particles[4].springs.push_back(sp);

	updateState();
}

void PendulumSystem::setState(const std::vector<Vector3f>& newState)
{
	m_vVecState = newState;
	int index = 0;
	for (int i = 0; i < m_vVecState.size(); i+=2)
	{
		particles[index].pos = m_vVecState[i];
		particles[index].vel = m_vVecState[i + VEL_OFFSET];
	}
}

void PendulumSystem::updateState()
{
	for (int i = 0; i < particles.size(); i++)
	{
		m_vVecState.push_back(particles[i].pos);
		m_vVecState.push_back(particles[i].vel);
	}
}


std::vector<Vector3f> PendulumSystem::evalF(std::vector<Vector3f> state)
{
	std::vector<Vector3f> f;
	// TODO 4.1: implement evalF
	//  - gravity
	//  - viscous drag
	//  - springs
	for (int i = 0; i < state.size(); i += 2)
	{
		Vector3f pos = state[i];
		Vector3f vel = state[i + VEL_OFFSET];
		Particle part = particles[i / 2];

		Vector3f F = Vector3f(0, part.mass * -G, 0) + vel * -DRAG;

		for (Spring sp : part.springs)
		{
			Vector3f D = pos - state[sp.index * 2];
			F += -sp.sturdiness * (D.abs() - sp.restLen) * (D / D.abs());
		}

		if (part.mass != 0.0f)
		{
			F = F / part.mass;
		}

		f.push_back(vel);
		f.push_back(F);
	}


	return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw(GLProgram& gl)
{
	const Vector3f PENDULUM_COLOR(0.73f, 0.0f, 0.83f);
	gl.updateMaterial(PENDULUM_COLOR);

	// TODO 4.2, 4.3

	// example code. Replace with your own drawing  code

	for (int i = 0; i < m_vVecState.size(); i += 2)
	{
		Vector3f pos = m_vVecState[i];
		gl.updateModelMatrix(Matrix4f::translation(pos));
		drawSphere(0.075f, 10, 10);
	}
}
