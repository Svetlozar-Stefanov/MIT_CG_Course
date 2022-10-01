#include "pendulumsystem.h"

#include <cassert>
#include "camera.h"
#include "vertexrecorder.h"

// TODO adjust to number of particles.
const int NUM_PARTICLES = 10;
const float G = 0.5f;
const float DRAG = 0.4f;

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
		part.pos = Vector3f(rand_uniform((float)i, 3), 0, 0);
		part.vel = Vector3f(0, rand_uniform(-5, 5), 0);
		part.mass = rand_uniform(0.5, 1);
		particles.push_back(part);
	}

	Particle end2;
	end2.pos = Vector3f(rand_uniform(3, 8), 2, 0);
	end2.vel = Vector3f(0, 0, 0);
	end2.mass = 0;
	particles.push_back(end2);

	for (int i = 0; i < particles.size() - 1; i++)
	{
		Spring newSpring;
		newSpring.index1 = i;
		newSpring.index2 = i + 1;

		newSpring.restLen = 0.2f;
		newSpring.sturdiness = 1;
		springs.push_back(newSpring);
	}

	updateState();
}

void PendulumSystem::setState(const std::vector<Vector3f>& newState)
{
	m_vVecState = newState;
	int index = 0;
	for (int i = 0; i < m_vVecState.size(); i += 2)
	{
		particles[index].pos = m_vVecState[i];
		particles[index].vel = m_vVecState[i + VEL_OFFSET];
		index++;
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
	std::vector<Vector3f> f(state.size());
	// TODO 4.1: implement evalF
	//  - gravity
	//  - viscous drag
	//  - springs
	for (int i = 0; i < springs.size(); i++)
	{
		Spring sp = springs[i];

		int pi1 = sp.index1 * 2;
		int pi2 = sp.index2 * 2;

		Vector3f pos1 = state[pi1];
		Vector3f vel1 = state[pi1 + VEL_OFFSET];
		Particle part1 = particles[sp.index1];

		Vector3f pos2 = state[pi2];
		Vector3f vel2 = state[pi2 + VEL_OFFSET];
		Particle part2 = particles[sp.index2];

		Vector3f D = pos1 - pos2;
		Vector3f sF = -sp.sturdiness * (D.abs() - sp.restLen) * (D / D.abs());
		f[pi1 + VEL_OFFSET] += sF;
		D = pos2 - pos1;
		sF = -sp.sturdiness * (D.abs() - sp.restLen) * (D / D.abs());
		f[pi2 + VEL_OFFSET] += sF;

	}

	for (int i = 0; i < state.size(); i += 2)
	{
		Vector3f pos = state[i];
		Vector3f vel = state[i + VEL_OFFSET];
		Particle part = particles[i / 2];

		Vector3f F = Vector3f(0, part.mass * -G, 0) + vel * -DRAG;
		f[i + VEL_OFFSET] += F;

		if (part.mass != 0.0f)
		{
			f[i + VEL_OFFSET] = f[i + VEL_OFFSET] / part.mass;
		}
		else
		{
			f[i + VEL_OFFSET] = Vector3f(0, 0, 0);
		}

		f[i] = vel;
	}

	return f;
}

// render the system (ie draw the particles)
void PendulumSystem::draw(GLProgram& gl, bool showSprings)
{
	const Vector3f PENDULUM_COLOR(0.73f, 0.0f, 0.83f);
	gl.updateMaterial(PENDULUM_COLOR);

	for (int i = 0; i < particles.size(); i++)
	{
		Particle pt = particles[i];
		Vector3f pos = pt.pos;
		gl.updateModelMatrix(Matrix4f::translation(pos));
		drawSphere(0.075f, 10, 10);
	}

	if (showSprings)
	{
		for (int i = 0; i < springs.size(); i++)
		{
			Spring sp = springs[i];

			Vector3f pos1 = particles[sp.index1].pos;
			Vector3f pos2 = particles[sp.index2].pos;
			Vector3f dir = Vector3f(0,0,1);
			if (pos1.x() < pos2.x())
			{
				dir[2] = -dir[2];
			}

			float deg = acos(Vector3f::dot(Vector3f(0, 1, 0), (pos2 - pos1).normalized()));
			Matrix4f rot = Matrix4f::rotation(dir, deg);

			Matrix4f M = Matrix4f::translation(pos1) * rot;
			gl.updateModelMatrix(M);

			drawCylinder(6, 0.02f, (pos1 - pos2).abs());
		}
	}
}