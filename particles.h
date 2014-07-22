#ifndef PARTICLES_H
#define PARTICLES_H

#include "algebra.hpp"
#include <GL/glu.h>
#define MAX_PARTICLES 1000

struct Particle
{
  Particle()
    : pos(0,0,0)
    , vel(0,0,0)
    , scale(1)
    , life(0)
  {
  }

  public:
    Point3D pos;
    Vector3D vel;
    double scale;
    double life;
};

class emitter
{
  public:
    emitter(GLuint texID);
    virtual ~emitter();

    void emit(Point3D p, Vector3D v, double s, double time);
    void update(double dt);
    void render();
  protected:
  private:
    Particle* particles[MAX_PARTICLES];
    int num_living;
    GLuint textureID;
};

#endif // PARTICLES_H
