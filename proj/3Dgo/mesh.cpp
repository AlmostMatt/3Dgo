#include "mesh.hpp"
#include <iostream>

Mesh::Mesh(const std::vector<Point3D>& verts,
           const std::vector< std::vector<int> >& faces)
  : m_verts(verts),
    m_faces(faces)
{
    std::cerr << "Computing bounds for a polygon mesh\n";
    // determine a bounding box/sphere
    // whichever has a smaller volume
    double minx, maxx, miny, maxy, minz, maxz;
    double maxdd;
    Point3D c;
    for (std::vector<Point3D>::const_iterator I = m_verts.begin(); I != m_verts.end(); ++I) {
      Point3D p = *I;
      if (I == m_verts.begin()) {
        minx = p[0];
        miny = p[1];
        minz = p[2];
        maxx = p[0];
        maxy = p[1];
        maxz = p[2];
        maxdd = 0;
        c = p;
      }
      minx = std::min(p[0], minx);
      miny = std::min(p[1], miny);
      minz = std::min(p[2], minz);
      maxx = std::max(p[0], maxx);
      maxy = std::max(p[1], maxy);
      maxz = std::max(p[2], maxz);

      std::vector<Point3D>::const_iterator J = I;
      for (++J; J != m_verts.end(); ++J) {
        Point3D p2 = *J;
        double dist = (p2 - p).length2();
        if (dist > maxdd) {
            maxdd = dist;
            c = p + 0.5 * (p2 - p);
        }
      }
    }
    // for now use a sphere
    double r = sqrt(maxdd)/2.0;
    double spherevolume = 1.33 * M_PI * pow(r, 3.0);
    
    Point3D min = Point3D(minx, miny, minz);
    Point3D max = Point3D(maxx, maxy, maxz);
    Vector3D sz = max-min;
    // volume = w h d
    double boxvolume = sz[0] * sz[1] * sz[2];
    if (spherevolume < boxvolume) {
        std::cerr << "Sphere is smaller\n";
        m_bounds = new NonhierSphere(c, r);
    } else {
        std::cerr << "Box is smaller\n";
        m_bounds = new NonhierBox(min, sz);
    }
}

std::ostream& operator<<(std::ostream& out, const Mesh& mesh)
{
  std::cerr << "mesh({";
  for (std::vector<Point3D>::const_iterator I = mesh.m_verts.begin(); I != mesh.m_verts.end(); ++I) {
    if (I != mesh.m_verts.begin()) std::cerr << ",\n      ";
    std::cerr << *I;
  }
  std::cerr << "},\n\n     {";
  
  for (std::vector<Mesh::Face>::const_iterator I = mesh.m_faces.begin(); I != mesh.m_faces.end(); ++I) {
    if (I != mesh.m_faces.begin()) std::cerr << ",\n      ";
    std::cerr << "[";
    for (Mesh::Face::const_iterator J = I->begin(); J != I->end(); ++J) {
      if (J != I->begin()) std::cerr << ", ";
      std::cerr << *J;
    }
    std::cerr << "]";
  }
  std::cerr << "});" << std::endl;
  return out;
}

RayHit* Mesh::raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces)
{
  // use bounding box or sphere first
  RayHit* hit = m_bounds->raycast(from, ray, m_material, true); // if inside the bounding box, hitting the back of a face should still count
  //return hit;
  if (hit == NULL) return hit;
  free(hit);
  hit = NULL;
  for (std::vector<Mesh::Face>::const_iterator I = m_faces.begin(); I != m_faces.end(); ++I) {
    Mesh::Face::const_iterator J = I->begin();
    Point3D A = m_verts[*J];
    ++J;
    Point3D B = m_verts[*J];
    ++J;
    Point3D C = m_verts[*J];
    Vector3D norm = (B-A).cross(C-B);
    //norm.normalize();
    // back face cull, norm and ray should be facing each other
    if (norm.dot(ray) < 0 || backfaces)
    {
      // intersect the plane
      double t = ((A - from).dot(norm))/(ray.dot(norm));
      if (t > 0 && (hit == NULL || t < hit->dist)) {
        Point3D P = from + t * ray;
        A = m_verts[I->back()]; // if face is ABCD, do edge DA first then AB BC CD
        bool in = true;
        for (J = I->begin(); J != I->end(); ++J) {
          B = m_verts[*J];
          // consider edge BC
          Vector3D edgenorm = (B-A).cross(norm);
          edgenorm.normalize();
          if (edgenorm.dot(B-P) < -0.01)
          {
            in = false;
            break;
          }
          A = B;
        }
        if (in) {
        /*
          std::cerr << P << " is in [";
          for (J = I->begin(); J != I->end(); ++J) {
            if (J != I->begin()) std::cerr << ", ";
            std::cerr << m_verts[*J];
          }
          std::cerr << "]\n";
          */
          
          // this will memleak if unused (or even used) hits arent freed
          if (hit != NULL) {
            free(hit);
          }
          hit = new RayHit(t, P, m_material, norm); //this norm is not yet normalized and the distance is not accurate yet
        }
      }
    }
  }
  //return NULL;
  return hit;
}
