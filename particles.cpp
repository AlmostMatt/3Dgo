#include "particles.h"

emitter::emitter(GLuint texID)
  : num_living(0)
  , textureID(texID)
{
  // the camera is static, so compute the corresponding camera aligned vertices now
  for (int i=0; i<MAX_PARTICLES; i++) {
    particles[i] = new Particle();
  }
}

emitter::~emitter()
{
  //dtor
}

void emitter::emit(Point3D pos, Vector3D v, double s=1.0, double time=1.0, Colour col=Colour(1.0,1.0,1.0))
{
  if (num_living < MAX_PARTICLES) {
    Particle* p = particles[num_living];
    p->pos = pos;
    p->vel = v;
    p->life = time;
    p->scale = s;
    p->col = col;
    num_living++;
    //std::cerr << "New particle has position " << particles[num_living]->pos << " (" << p->pos << ")" << std::endl;
  }
}

void emitter::update(double dt)
{
  for (int i=0; i<num_living; i++) {
    Particle* p = particles[i];
    if (p->life > 0.0) {
      p->pos = p->pos + dt * p->vel;
      p->life -= dt;
    } else {
      num_living--;
      Particle* p2 = particles[num_living];
      p->pos = p2->pos;
      p->vel = p2->vel;
      p->life = p2->life;
      p->scale = p2->scale;
      p->col = p2->col;
      // copy the last particle to this position and ignore the old particle
      // it will not be rendered
    }
  }
}

void emitter::render()
{
  glDisable(GL_LIGHTING);

  glEnable(GL_BLEND);
  glDisable(GL_DEPTH_TEST);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  GLfloat white[] = {1.0, 1.0, 1.0, 1.0};
  GLfloat shiny[] = {1.0};

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, white);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shiny);

  // determine camera orientation? local XY
  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, textureID);
  glBegin(GL_QUADS);
  for (int i=0; i<num_living; i++) {
    Particle* p = particles[i];
    //std::cerr << "Particle is at position " << p->pos << std::endl;

    glColor4d(p->col.R(), p->col.G(), p->col.B(), p->life);
    double sx = 0.2 * p->scale;
    double sy = 0.2 * p->scale;
    double sz = 0.2 * p->scale;
    glNormal3f(0.0f,0.0f,-1.0f);
    glTexCoord2f(0.0, 0.0);
    glVertex3d(p->pos[0] - sx, p->pos[1] - sy, p->pos[2] + sz);
    glTexCoord2f(1.0, 0.0);
    glVertex3d(p->pos[0] + sx, p->pos[1] - sy, p->pos[2] + sz);
    glTexCoord2f(1.0, 1.0);
    glVertex3d(p->pos[0] + sx, p->pos[1] + sy, p->pos[2] - sz);
    glTexCoord2f(0.0, 1.0);
    glVertex3d(p->pos[0] - sx, p->pos[1] + sy, p->pos[2] - sz);
  }
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_BLEND);
  glEnable(GL_LIGHTING);
  glColor4d(1.0, 1.0, 1.0, 1.0); // don't tint future objects
}
