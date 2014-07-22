#ifndef CS488_MESH_HPP
#define CS488_MESH_HPP

#include <vector>
#include <iosfwd>
#include "primitive.hpp"
#include "algebra.hpp"

// A polygonal mesh.
class Mesh : public Primitive {
public:
  Mesh(const std::vector<Point3D>& verts,
       const std::vector< std::vector<int> >& faces);
  virtual RayHit* raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces = false);

  typedef std::vector<int> Face;
  
private:
  std::vector<Point3D> m_verts;
  std::vector<Face> m_faces;
  Primitive* m_bounds;

  friend std::ostream& operator<<(std::ostream& out, const Mesh& mesh);
};

#endif
