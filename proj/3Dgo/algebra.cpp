//---------------------------------------------------------------------------
//
// CS488 -- Introduction to Computer Graphics
//
// algebra.hpp/algebra.cpp
//
// Classes and functions for manipulating points, vectors, matrices,
// and colours.  You probably won't need to modify anything in these
// two files.
//
// University of Waterloo Computer Graphics Lab / 2003
//
//---------------------------------------------------------------------------

#include "algebra.hpp"

Vector3D Vector3D::scaleTo(double s)
{
    return (s/(length())) * (*this);
}


double Vector3D::normalize()
{
  double denom = 1.0 / length();
  // double denom = 1.0;
  // double x = (v_[0] > 0.0) ? v_[0] : -v_[0];
  // double y = (v_[1] > 0.0) ? v_[1] : -v_[1];
  // double z = (v_[2] > 0.0) ? v_[2] : -v_[2];

  // if(x > y) {
    // if(x > z) {
      // if(1.0 + x > 1.0) {
        // y = y / x;
        // z = z / x;
        // denom = 1.0 / (x * sqrt(1.0 + y*y + z*z));
      // }
    // } else { /* z > x > y */
      // if(1.0 + z > 1.0) {
        // y = y / z;
        // x = x / z;
        // denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
      // }
    // }
  // } else {
    // if(y > z) {
      // if(1.0 + y > 1.0) {
        // z = z / y;
        // x = x / y;
        // denom = 1.0 / (y * sqrt(1.0 + z*z + x*x));
      // }
    // } else { /* x < y < z */
      // if(1.0 + z > 1.0) {
        // y = y / z;
        // x = x / z;
        // denom = 1.0 / (z * sqrt(1.0 + y*y + x*x));
      // }
    // }
  // }


  // if(1.0 + x + y + z > 1.0) {
    v_[0] *= denom;
    v_[1] *= denom;
    v_[2] *= denom;
    return 1.0 / denom;
  // }
  return 0.0;
}

Vector3D Vector3D::proj(const Vector3D& other)
{
//return (dot(other)/other.length2()) * other;
// double s = abs((v_[0]*other.v_[0] + v_[1]*other.v_[1] + v_[2]*other.v_[2])
  // / (other.v_[0]*other.v_[0] + other.v_[1]*other.v_[1] + other.v_[2]*other.v_[2]));
// return Vector3D(s*other.v_[0], s*other.v_[1], s*other.v_[2]);
    return (dot(other)/other.length2()) * other;
}

// assume norm is unit
// and the ray and normal are roughly facing each other
Vector3D Vector3D::reflect(const Vector3D& norm )
{
  Vector3D r2 = -1.0 * (*this);
  return r2 - (2 * (r2.dot(norm))) * norm;
}

/*
 * Define some helper functions for matrix inversion.
 */

static void swaprows(Matrix4x4& a, size_t r1, size_t r2)
{
  std::swap(a[r1][0], a[r2][0]);
  std::swap(a[r1][1], a[r2][1]);
  std::swap(a[r1][2], a[r2][2]);
  std::swap(a[r1][3], a[r2][3]);
}

static void dividerow(Matrix4x4& a, size_t r, double fac)
{
  a[r][0] /= fac;
  a[r][1] /= fac;
  a[r][2] /= fac;
  a[r][3] /= fac;
}

static void submultrow(Matrix4x4& a, size_t dest, size_t src, double fac)
{
  a[dest][0] -= fac * a[src][0];
  a[dest][1] -= fac * a[src][1];
  a[dest][2] -= fac * a[src][2];
  a[dest][3] -= fac * a[src][3];
}

/*
 * invertMatrix
 *
 * I lifted this code from the skeleton code of a raytracer assignment
 * from a different school.  I taught that course too, so I figured it
 * would be okay.
 */
Matrix4x4 Matrix4x4::invert() const
{
  /* The algorithm is plain old Gauss-Jordan elimination
     with partial pivoting. */

  Matrix4x4 a(*this);
  Matrix4x4 ret;

  /* Loop over cols of a from left to right,
     eliminating above and below diag */

  /* Find largest pivot in column j among rows j..3 */
  for(size_t j = 0; j < 4; ++j) {
    size_t i1 = j; /* Row with largest pivot candidate */
    for(size_t i = j + 1; i < 4; ++i) {
      if(fabs(a[i][j]) > fabs(a[i1][j])) {
        i1 = i;
      }
    }

    /* Swap rows i1 and j in a and ret to put pivot on diagonal */
    swaprows(a, i1, j);
    swaprows(ret, i1, j);

    /* Scale row j to have a unit diagonal */
    if(a[j][j] == 0.0) {
      // Theoretically throw an exception.
      return ret;
    }

    dividerow(ret, j, a[j][j]);
    dividerow(a, j, a[j][j]);

    /* Eliminate off-diagonal elems in col j of a, doing identical
       ops to b */
    for(size_t i = 0; i < 4; ++i) {
      if(i != j) {
        submultrow(ret, i, j, a[i][j]);
        submultrow(a, i, j, a[i][j]);
      }
    }
  }

  return ret;
}


Matrix4x4 rotation(char axis, double angle)
{
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
  return r;
}

Matrix4x4 scaling(const Vector3D& amount)
{
  Matrix4x4 s = Matrix4x4(Vector4D(amount[0], 0, 0, 0),
                          Vector4D(0, amount[1], 0, 0),
                          Vector4D(0, 0, amount[2], 0),
                          Vector4D(0, 0, 0, 1));
  return s;
}

Matrix4x4 translation(const Point3D& amount)
{
  Matrix4x4 t = Matrix4x4(Vector4D(1, 0, 0, amount[0]),
                          Vector4D(0, 1, 0, amount[1]),
                          Vector4D(0, 0, 1, amount[2]),
                          Vector4D(0, 0, 0, 1));
  return t;
}

// the plane is a 2d parallelogram defined by a point and 2 vectors
// the sphere is an ellipsoid
// the norm is towards the sphere and away from the plane
Intersection spherePlane(Point3D spherepos, Vector3D spherescale, Point3D pt1, Vector3D v1, Vector3D v2) {
  Vector3D norm = v1.cross(v2);
  Point3D onPlane = spherepos - ((spherepos - pt1).proj(norm));
  //
  double dist = ((spherepos - onPlane) / spherescale).length2();
  if (dist < 1.0) { // unti radius
    return Intersection(onPlane, (spherepos - onPlane)/spherescale, 1 - sqrt(dist));
  }
  // find the distance to the plane
  // or to the nearest edge/corner
  return Intersection();
}

// assumes that the box is axis aligned
Intersection sphereBox(Point3D spherepos, Vector3D spherescale, Point3D boxpos, Vector3D boxsize)
{
  // find the distance along each axis to the sphere ( or 0 for any axis where the sphere is between the min and max)
  double dd = 0.0;
  Point3D pt = spherepos;
  for (int axis = 0; axis < 3; axis++) {
    if (spherepos[axis] < boxpos[axis]) {
      pt[axis] = boxpos[axis];
    } else if (spherepos[axis] > boxpos[axis] + boxsize[axis]) {
      pt[axis] = boxpos[axis] + boxsize[axis];
    }
  }
  double dist = ((pt - spherepos) / spherescale).length2();
  if (dist < 1.0) {
    return Intersection(pt, (spherepos - pt)/spherescale, 1 - sqrt(dist));
  }
  return Intersection();
}

// assumes that both spheres have the same scale and orientation
// the normal will be facing towards pos1
Intersection sphereSphere(Point3D pos1, Point3D pos2, Vector3D spherescale)
{
  // find the distance along each axis to the sphere ( or 0 for any axis where the sphere is between the min and max)
  double dist = ((pos1 - pos2) / spherescale).length2();
  if (dist < 4.0) { // two unit spheres have combined radius 2
    return Intersection(pos1.avg(pos2), (pos1 - pos2)/spherescale, (2.0 - sqrt(dist))/2.0);
  }
  return Intersection();
}
