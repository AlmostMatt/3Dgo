#include "scene.hpp"
#include <iostream>

SceneNode::SceneNode(const std::string& name, bool dyn)
  : m_name(name)
  , dynamic(dyn)
{
}

void SceneNode::getobjectlist(std::list<Object*>& objects, Matrix4x4 t, Matrix4x4 ti, Matrix4x4 tit)
{
  Matrix4x4 newt = t * get_transform();
  Matrix4x4 newti = get_inverse() * ti;
  Matrix4x4 newtit = (get_inverse().transpose()) * tit;
  for (ChildList::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->getobjectlist(objects, newt, newti, newtit);
  }
}

RayHit* SceneNode::raycast(Point3D from, Vector3D ray)
{
  RayHit* result = NULL;
  //glMultMatrixd(get_transform().transpose().begin());
  for (ChildList::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    // record current matrix transform
    //glPushMatrix();
    RayHit* hit = (*it)->raycast(from, ray);
    if (hit != NULL && (result == NULL || result->dist > hit->dist))
      result = hit;
    //revert to previous matrix transform
    //glPopMatrix();
  }
  return result;
}

SceneNode::~SceneNode()
{
}

void SceneNode::rotate(char axis, double angle)
{
  std::cerr << "Rotate " << m_name << " around " << axis << " by " << angle << std::endl;
  if (axis == 'y')
  {
    angle = -angle;
  }
  double cosa = cos(angle * M_PI / 180);
  double sina = sin(angle * M_PI / 180);
  Matrix4x4 r; // the identity matrix
  if (axis == 'x') {
    r = Matrix4x4(Vector4D(1, 0, 0, 0),
                  Vector4D(0, cosa, -sina, 0),
                  Vector4D(0, sina, cosa, 0),
                  Vector4D(0, 0, 0, 1));
  } else if (axis == 'y') {
    r = Matrix4x4(Vector4D(cosa, 0, -sina, 0),
                  Vector4D(0, 1, 0, 0),
                  Vector4D(sina, 0, cosa, 0),
                  Vector4D(0, 0, 0, 1));
  } else if (axis == 'z') {
    r = Matrix4x4(Vector4D(cosa, -sina, 0, 0),
                  Vector4D(sina, cosa, 0, 0),
                  Vector4D(0, 0, 1, 0),
                  Vector4D(0, 0, 0, 1));
  }
  set_transform(get_transform() * r);
}

void SceneNode::scale(const Vector3D& amount)
{
  std::cerr << "Scale " << m_name << " by " << amount << std::endl;
  Matrix4x4 s = Matrix4x4(Vector4D(amount[0], 0, 0, 0),
                          Vector4D(0, amount[1], 0, 0),
                          Vector4D(0, 0, amount[2], 0),
                          Vector4D(0, 0, 0, 1));
  set_transform(get_transform() * s);
}

void SceneNode::translate(const Vector3D& amount)
{
  std::cerr << "Translate " << m_name << " by " << amount << std::endl;
  Matrix4x4 t = Matrix4x4(Vector4D(1, 0, 0, amount[0]),
                          Vector4D(0, 1, 0, amount[1]),
                          Vector4D(0, 0, 1, amount[2]),
                          Vector4D(0, 0, 0, 1));
  set_transform(get_transform() * t);
}

bool SceneNode::is_joint() const
{
  return false;
}

JointNode::JointNode(const std::string& name, bool dyn)
  : SceneNode(name, dyn)
{
}

JointNode::~JointNode()
{
}

bool JointNode::is_joint() const
{
  return true;
}

void JointNode::set_joint_x(double min, double init, double max)
{
  m_joint_x.min = min;
  m_joint_x.init = init;
  m_joint_x.max = max;
}

void JointNode::set_joint_y(double min, double init, double max)
{
  m_joint_y.min = min;
  m_joint_y.init = init;
  m_joint_y.max = max;
}

GeometryNode::GeometryNode(const std::string& name, Primitive* primitive, bool dyn)
  : SceneNode(name, dyn)
  , m_primitive(primitive)
{
}

void GeometryNode::getobjectlist(std::list<Object*>& objects, Matrix4x4 t, Matrix4x4 ti, Matrix4x4 tit)
{
  Matrix4x4 newt = t * get_transform();
  Matrix4x4 newti = get_inverse() * ti;
  Matrix4x4 newtit = (get_inverse().transpose()) * tit;
  for (ChildList::const_iterator it = m_children.begin(); it != m_children.end(); it++) {
    (*it)->getobjectlist(objects, newt, newti, newtit);
  }
  objects.push_back(new Object(m_name, newt, newti, newtit, m_material, m_primitive, dynamic));
}

RayHit* GeometryNode::raycast(Point3D from, Vector3D ray)
{
  RayHit* hit1 = SceneNode::raycast(from, ray);
  RayHit* hit2 = m_primitive->raycast(from, ray, m_material);
  //std::cerr << "Hit1: " << hit1 << " Hit2: " << hit2 << ".\n";
  //m_material->apply_gl(m_selected);
  if (hit1 == NULL)
    return hit2;
  else if (hit2 == NULL)
    return hit1;
  else if (hit1->dist < hit2->dist)
    return hit1;
  else
    return hit2;
}

GeometryNode::~GeometryNode()
{
}

