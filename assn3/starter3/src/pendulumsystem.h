#ifndef PENDULUMSYSTEM_H
#define PENDULUMSYSTEM_H

#include <vector>

#include "particlesystem.h"

class PendulumSystem : public ParticleSystem
{
public:
    PendulumSystem();

    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

    /*std::vector<Vector3f> getState() override;*/

    void setState(const std::vector<Vector3f>& newState) override;

    void draw(GLProgram&);

private:
    std::vector<Particle> particles;
    std::vector<Spring> springs;
    void updateState();
    // inherits 
    // std::vector<Vector3f> m_vVecState;
};

#endif
