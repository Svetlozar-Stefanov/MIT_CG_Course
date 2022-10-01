#include "clothsystem.h"
#include "camera.h"
#include "vertexrecorder.h"

// your system should at least contain 8x8 particles.
const int W = 6;
const int H = 6;

const float G = 0.5f;
const float DRAG = 3;

const int VEL_OFFSET = 1;

ClothSystem::ClothSystem()
{
	// TODO 5. Initialize m_vVecState with cloth particles. 
	// You can again use rand_uniform(lo, hi) to make things a bit more interesting

	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			Particle pt;
			pt.pos = Vector3f(rand_uniform(-5 + (float)j * 2, -4 + (float)j * 2), rand_uniform(5 - (float)i * 2 , 6 - (float)i * 2), 0);
			if (i == 0 && (j == 0 || j == W - 1))
			{
				pt.vel == Vector3f(0, 0, 0);
				pt.mass = 0;
			}
			else
			{
				pt.vel = Vector3f(0, rand_uniform(-5, 0), 0);
				pt.mass = rand_uniform(0.5, 1);
			}

			particles.push_back(pt);
		}
	}

	for (int i = 0; i < H; i++)
	{
		for (int j = 0; j < W; j++)
		{
			//Structural Springs
			if (j < W - 1)
			{
				Spring newSpring;
				newSpring.index1 = IndexOf(i, j);
				newSpring.index2 = IndexOf(i, j + 1);
				newSpring.restLen = 1;
				newSpring.sturdiness = 3;
				springs.push_back(newSpring);
			}

			if (i > 0)
			{
				Spring newSpring;
				newSpring.index1 = IndexOf(i, j);
				newSpring.index2 = IndexOf(i - 1, j);
				newSpring.restLen = 1;
				newSpring.sturdiness = 3;
				springs.push_back(newSpring);
			}

			//Shear Springs
			int currentIndex = IndexOf(i, j);
			int rightIndex = -1;
			int leftIndex = -1;
			if (i + 1 < H && j - 1 >= 0)
			{
				leftIndex = IndexOf(i + 1, j - 1);
			}
			if (i + 1 < H && j + 1 < W)
			{
				rightIndex = IndexOf(i + 1, j + 1);
			}

			if (rightIndex != -1)
			{
				Spring newSpring;
				newSpring.index1 = currentIndex;
				newSpring.index2 = rightIndex;
				newSpring.restLen = 2;
				newSpring.sturdiness = 1;
				springs.push_back(newSpring);
			}
			if (leftIndex != -1)
			{
				Spring newSpring;
				newSpring.index1 = currentIndex;
				newSpring.index2 = leftIndex;
				newSpring.restLen = 2;
				newSpring.sturdiness = 1;
				springs.push_back(newSpring);
			}

			//Flex Springs
			currentIndex = IndexOf(i, j);
			rightIndex = -1;
			int downIndex = -1;

			if (j + 2 < W)
			{
				rightIndex = IndexOf(i, j + 2);
			}
			if (i + 2 < H)
			{
				downIndex = IndexOf(i + 2, j);
			}

			if (downIndex != -1)
			{
				Spring newSpring;
				newSpring.index1 = currentIndex;
				newSpring.index2 = downIndex;
				newSpring.restLen = 0.6f;
				newSpring.sturdiness = 1;
				springs.push_back(newSpring);
			}
			if (rightIndex != -1)
			{
				Spring newSpring;
				newSpring.index1 = currentIndex;
				newSpring.index2 = rightIndex;
				newSpring.restLen = 0.6f;
				newSpring.sturdiness = 1;
				springs.push_back(newSpring);
			}
		}
	}

	updateState();
}

void ClothSystem::setState(const std::vector<Vector3f>& newState)
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

void ClothSystem::updateState()
{
	for (int i = 0; i < particles.size(); i++)
	{
		m_vVecState.push_back(particles[i].pos);
		m_vVecState.push_back(particles[i].vel);
	}
}

int ClothSystem::IndexOf(const int i, const int j)
{
	return i * W + j;
}

std::vector<Vector3f> ClothSystem::evalF(std::vector<Vector3f> state)
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


void ClothSystem::draw(GLProgram& gl)
{
	//TODO 5: render the system 
	//         - ie draw the particles as little spheres
	//         - or draw the springs as little lines or cylinders
	//         - or draw wireframe mesh

	const Vector3f CLOTH_COLOR(0.9f, 0.9f, 0.9f);
	gl.updateMaterial(CLOTH_COLOR);

	//// EXAMPLE for how to render cloth particles.
	////  - you should replace this code.
	//float w = 0.2f;
	//Vector3f O(0.4f, 1, 0);
	//gl.updateModelMatrix(Matrix4f::translation(O));
	//drawSphere(0.04f, 8, 8);
	//gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(w, 0, 0)));
	//drawSphere(0.04f, 8, 8);
	//gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(w, -w, 0)));
	//drawSphere(0.04f, 8, 8);
	//gl.updateModelMatrix(Matrix4f::translation(O + Vector3f(0, -w, 0)));
	//drawSphere(0.04f, 8, 8);

	//// EXAMPLE: This shows you how to render lines to debug the spring system.
	////
	////          You should replace this code.
	////
	////          Since lines don't have a clearly defined normal, we can't use
	////          a regular lighting model.
	////          GLprogram has a "color only" mode, where illumination
	////          is disabled, and you specify color directly as vertex attribute.
	////          Note: enableLighting/disableLighting invalidates uniforms,
	////          so you'll have to update the transformation/material parameters
	////          after a mode change.
	//gl.disableLighting();
	//gl.updateModelMatrix(Matrix4f::identity()); // update uniforms after mode change
	//VertexRecorder rec;
	//rec.record(O, CLOTH_COLOR);
	//rec.record(O + Vector3f(w, 0, 0), CLOTH_COLOR);
	//rec.record(O, CLOTH_COLOR);
	//rec.record(O + Vector3f(0, -w, 0), CLOTH_COLOR);

	//rec.record(O + Vector3f(w, 0, 0), CLOTH_COLOR);
	//rec.record(O + Vector3f(w, -w, 0), CLOTH_COLOR);

	//rec.record(O + Vector3f(0, -w, 0), CLOTH_COLOR);
	//rec.record(O + Vector3f(w, -w, 0), CLOTH_COLOR);
	//glLineWidth(3.0f);
	//rec.draw(GL_LINES);

	//gl.enableLighting(); // reset to default lighting model
	//// EXAMPLE END

	for (int i = 0; i < particles.size(); i++)
	{
		Particle pt = particles[i];
		Vector3f pos = pt.pos;
		gl.updateModelMatrix(Matrix4f::translation(pos));
		drawSphere(0.075f, 10, 10);
	}

	for (int i = 0; i < springs.size(); i++)
	{
		Spring sp = springs[i];

		Vector3f pos1 = particles[sp.index1].pos;
		Vector3f pos2 = particles[sp.index2].pos;
		Vector3f dir = Vector3f(0, 0, 1);
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

