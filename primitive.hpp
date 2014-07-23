#ifndef CS488_PRIMITIVE_HPP
#define CS488_PRIMITIVE_HPP

#include "algebra.hpp"
#include "material.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include "SDL_OGL.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <list>

#define GRAVITY -20.0
#define FRICTION 50.0
#define TERMINAL 20.0

#define BOARD 13
#include "SoundManager.h"

class RayHit {
public:
  RayHit(double d, Point3D p, Material* m, Vector3D n);
  double dist;
  Point3D pos;
  Material* mat;
  Vector3D norm;
};

class Primitive {
public:
  virtual ~Primitive();
  virtual void render();
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);
};

class Object {
public:
  Object(std::string name, Point3D pos, Vector3D scale, Material* m, Primitive* p, bool dyn);
  RayHit* raycast(Point3D from, Vector3D ray);
  void render();
  void seek(Point3D point, double accel);
  void move(double dt, std::list<Object*>& objects);

  std::string m_name;
  Matrix4x4 transform;
  bool dynamic;
  Point3D m_pos;
  Vector3D m_vel;
  Vector3D m_scale;
  Primitive* m_primitive;
  Material* m_material;
private:
  Matrix4x4 inversetransform;
};

class Sphere : public Primitive {
public:
  Sphere();
  virtual ~Sphere();
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);
  virtual void render();
private:
  GLUquadricObj* quadric;
};

class Cube : public Primitive {
public:
  virtual ~Cube();
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);
  virtual void render();
};

class NonhierSphere : public Primitive {
public:
  NonhierSphere(const Point3D& pos, double radius)
    : m_pos(pos), m_radius(radius), quadric(gluNewQuadric())
  {
  }
  virtual ~NonhierSphere();
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);
  virtual void render();

private:
  GLUquadricObj* quadric;
  Point3D m_pos;
  double m_radius;
};

class NonhierBox : public Primitive {
public:
  NonhierBox(const Point3D& pos, double size)
    : m_pos(pos), m_size(size, size, size), hasTexture(false)
  {
  }

  NonhierBox(const Point3D& pos, Vector3D size)
    : m_pos(pos), m_size(size), hasTexture(false)
  {
  }

  NonhierBox(const Point3D& pos, Vector3D size, GLuint textureID, GLuint bumpProgram, GLuint bumpMap)
    : m_pos(pos), m_size(size), hasTexture(true), textureID(textureID),bumpProgram(bumpProgram), bumpMap(bumpMap)
  {
  }

  virtual ~NonhierBox();
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);
  virtual void render();

  Point3D m_pos;
  Vector3D m_size;
private:
  bool hasTexture;
  GLuint textureID;
  GLuint bumpProgram;
  GLuint bumpMap;
};

#endif
