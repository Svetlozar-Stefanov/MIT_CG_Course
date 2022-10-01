#ifndef CLOTHSYSTEM_H
#define CLOTHSYSTEM_H

#include <vector>

#include "particlesystem.h"

class ClothSystem : public ParticleSystem
{
    ///ADD MORE FUNCTION AND FIELDS HERE
public:
    ClothSystem();

    void setState(const std::vector<Vector3f>& newState) override;

    // evalF is called by the integrator at least once per time step
    std::vector<Vector3f> evalF(std::vector<Vector3f> state) override;

    // draw is called once per frame
    void draw(GLProgram& ctx);

protected:
    std::vector<Particle> particles;
    std::vector<Spring> springs;
    void updateState();
    int IndexOf(const int i, const int j);


    // inherits
    // std::vector<Vector3f> m_vVecState;
};


#endif
