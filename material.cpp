#include "material.hpp"
#include <GL/gl.h>

Material::~Material()
{
}

PhongMaterial::PhongMaterial(const Colour& kd, const Colour& ks, double shininess)
  : m_kd(kd), m_ks(ks), m_shininess(shininess)
{
}

PhongMaterial::~PhongMaterial()
{
}

void PhongMaterial::apply_gl(bool selected, double alpha) const
{
  GLfloat mat_diff[] = {m_kd.R(),m_kd.G(),m_kd.B(),alpha};
  GLfloat mat_spec[] = {m_ks.R(),m_ks.G(),m_ks.B(),alpha};
  GLfloat mat_shiny[] = {m_shininess};

  //glEnable(GL_COLOR_MATERIAL);
  //glColorMaterial(GL_FRONT, GL_DIFFUSE);
  if (selected) {
    mat_diff[0] = mat_diff[0]/2;
    mat_diff[1] = mat_diff[1]/2;
    mat_diff[2] = mat_diff[2]/2;
  /*
    glColor3f(m_kd.R() /2, m_kd.G() /2, m_kd.B()/2);
  } else {
    glColor3f(m_kd.R(), m_kd.G(), m_kd.B());
  */
  }

  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, mat_diff);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_spec);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shiny);
}
