#include "primitive.hpp"

RayHit::RayHit(double d, Point3D p, Material* m, Vector3D n)
  : dist(d)
  , pos(p)
  , mat(m)
  , norm(n)
{
}

Object::Object(std::string name, Matrix4x4 t, Matrix4x4 ti, Matrix4x4 tit, Material* m, Primitive* p, bool dyn)
  : m_name(name)
  , transform(t)
  , inversetransform(ti)
  , inversetransposed(tit)
  , m_material(m)
  , m_primitive(p)
  , dynamic(dyn)
{
}

void Object::render() {
  m_material->apply_gl(false);
  glPushMatrix();
  glMultMatrixd(transform.transpose().begin());
  m_primitive->render();
  glPopMatrix();
}

RayHit* Object::raycast(Point3D from, Vector3D ray) {
  //std::cerr << "Object raycast: " << from << " in direction " << ray << "\n";
  // transform from and ray
  Vector3D newray = inversetransform * ray;
  //newray.normalize();
  RayHit* hit = m_primitive->raycast(inversetransform * from, newray, m_material);
  if (hit != NULL)
  {
    // inverse transform the normal and the hit point
    hit->pos = transform * hit->pos;
    hit->norm = transNorm(inversetransform, hit->norm); // THIS MIGHT BE WRONG
    hit->dist = (hit->pos - from).length(); // scaled objects shouldn't have scaled distances
    // I'm not certain what matrix transNorm expects me to provide, currently I give it the product of transposed inverted matrices, it may just expect a product of inverses and do some transposing on it's own
    //std::cerr << "Hit something\n";
  }
  return hit;
}

void Primitive::render() {
    return;
}

// generate a mesh for every primitive (cube etc)
// and just do triangle/quad intersection
RayHit* Primitive::raycast(Point3D from, Vector3D ray, Material* m_materia, bool backfaces) {
  return NULL;
}

Primitive::~Primitive()
{
}

Sphere::Sphere()
  : quadric(gluNewQuadric())
{
}

void Sphere::render() {
  gluQuadricNormals(quadric, GLU_SMOOTH);
  gluSphere(quadric, 1.0, 32, 32);
}

Sphere::~Sphere()
{
  gluDeleteQuadric(quadric);
}

RayHit* Sphere::raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces)
{
  Vector3D diff = Point3D(0,0,0) - from;
  // if the sphere's center is behind this point, ignore it as it can only give the wrong normal
  if (diff.dot(ray) < 0)
    return NULL;
  Vector3D A = diff.proj(ray);
  Vector3D B = diff - A;
  double b2 = B.length2();
  if (b2 < 1.001) {
    double a = A.length();
    double c = sqrt(1 - b2);
    double dist = a - c;
    if (dist <= 0.01 && backfaces) {
        dist = a + c;
    }
    //std::cerr << "Hit sphere, distance: " << dist << ".\n";
    if (dist > 0)
    {
      Point3D pt = from + ray.scaleTo(dist);
      Vector3D norm = pt - Point3D(0,0,0);
      // normalize normals later
      RayHit* result = new RayHit(dist, pt, m_material, norm);
      return result;
    }
  }
  return NULL;
}

void Cube::render() {
  glBegin(GL_QUADS);
  glNormal3f(-1.0f,0.0f,0.0f);
  glVertex3f(0.0f,0.0f,0.0f);
  glVertex3f(0.0f,0.0f,1.0f);
  glVertex3f(1.0f,0.0f,1.0f);
  glVertex3f(1.0f,0.0f,0.0f);

  glNormal3f(0.0f,0.0f,-1.0f);
  glVertex3f(0.0f,0.0f,0.0f);
  glVertex3f(1.0f,0.0f,0.0f);
  glVertex3f(1.0f,1.0f,0.0f);
  glVertex3f(0.0f,1.0f,0.0f);

  glNormal3f(1.0f,0.0f,0.0f);
  glVertex3f(1.0f,1.0f,0.0f);
  glVertex3f(1.0f,1.0f,1.0f);
  glVertex3f(0.0f,1.0f,1.0f);
  glVertex3f(0.0f,1.0f,0.0f);

  glNormal3f(0.0f,0.0f,1.0f);
  glVertex3f(1.0f,1.0f,1.0f);
  glVertex3f(1.0f,0.0f,1.0f);
  glVertex3f(0.0f,0.0f,1.0f);
  glVertex3f(0.0f,1.0f,1.0f);

  glNormal3f(0.0f,1.0f,0.0f);
  glVertex3f(0.0f,0.0f,0.0f);
  glVertex3f(0.0f,1.0f,0.0f);
  glVertex3f(0.0f,1.0f,1.0f);
  glVertex3f(0.0f,0.0f,1.0f);

  glNormal3f(0.0f,-1.0f,0.0f);
  glVertex3f(1.0f,0.0f,0.0f);
  glVertex3f(1.0f,0.0f,1.0f);
  glVertex3f(1.0f,1.0f,1.0f);
  glVertex3f(1.0f,1.0f,0.0f);
  glEnd();
}

RayHit* Cube::raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces)
{
    double tminx, tminy, tminz, tmaxx, tmaxy, tmaxz;
    // want to find min/max t values for the portion of the ray inside the cube along each axis
    // apparently negative 0 is a value and this will fail for it, but w/e
    // c division by 0 gives an infinite value, that should give correct results for < > min and max
    if (ray[0] >= 0) {
        tminx = (0 - from[0])/ray[0];
        tmaxx = (1 - from[0])/ray[0];
    } else {
        tminx = (1 - from[0])/ray[0];
        tmaxx = (0 - from[0])/ray[0];
    }
    if (ray[1] >= 0) {
        tminy = (0 - from[1])/ray[1];
        tmaxy = (1 - from[1])/ray[1];
    } else {
        tminy = (1 - from[1])/ray[1];
        tmaxy = (0 - from[1])/ray[1];
    }
    if (ray[2] >= 0) {
        tminz = (0 - from[2])/ray[2];
        tmaxz = (1 - from[2])/ray[2];
    } else {
        tminz = (1 - from[2])/ray[2];
        tmaxz = (0 - from[2])/ray[2];
    }
    double tmin = std::max(tminx, std::max(tminy, tminz));
    double tmax = std::min(tmaxx, std::min(tmaxy, tmaxz));
    /*
    if (tmin < 0.01 && backfaces && tmax > 0.01) {
        // this isn't actually an accurate point, but it will return a hit.
        Point3D pt = from + tmax * ray;
        Vector3D norm; // the normal doesn't have to be normalized yet
        if (tmax == tmaxx) {
            norm = Vector3D(-ray[0], 0, 0);
        } else if (tmax == tmaxy) {
            norm = Vector3D(0, -ray[1], 0);
        } else if (tmax == tmaxz) {
            norm = Vector3D(0, 0, -ray[2]);
        } else {
            // this should not be possible
            std::cerr << "!!!!! A backface cube normal is bad\n";
            norm = pt - Point3D(0,0,0);
        }
        // dist is not relevant here since it will be determined again after inverse transformations have happened
        return new RayHit((pt - from).length(), pt, m_material, norm);
    } else if (tmin < 0.01 || tmin > tmax + 0.01) {
        // tmin < 0 means the view is inside the cube
        // tmin > tmax means that the ray will miss the cube
        return NULL;

        the 0.01 seems to have made things worse in the above, probably because the ray is not normalized
       */
    if ( tmin < 0 && backfaces && tmax > 0) {
        Point3D pt = from + tmax * ray;
        Vector3D norm; // the normal doesn't have to be normalized yet
        if (tmax == tmaxx) {
            norm = Vector3D(-ray[0], 0, 0);
        } else if (tmax == tmaxy) {
            norm = Vector3D(0, -ray[1], 0);
        } else if (tmax == tmaxz) {
            norm = Vector3D(0, 0, -ray[2]);
        } else {
            // this should not be possible
            std::cerr << "!!!!! A backface nh_box normal is bad\n";
            norm = pt - Point3D(0,0,0);
        }
    } else if (tmin < 0 || tmin > tmax) {
        return NULL;
    } else {
        //std::cerr << "Hit a unit cube! From: " << from << " Ray: " << ray << "\n";
        Point3D pt = from + tmin * ray;
        Vector3D norm; // the normal doesn't have to be normalized yet
        if (tmin == tminx) {
            norm = Vector3D(-ray[0], 0, 0);
        } else if (tmin == tminy) {
            norm = Vector3D(0, -ray[1], 0);
        } else if (tmin == tminz) {
            norm = Vector3D(0, 0, -ray[2]);
        } else {
            // this should not be possible
            std::cerr << "!!!!! A cube normal is bad\n";
            norm = pt - Point3D(0,0,0);
        }
        // dist is not relevant here since it will be determined again after inverse transformations have happened
        return new RayHit((pt - from).length(), pt, m_material, norm);
    }
}

Cube::~Cube()
{
}

NonhierSphere::~NonhierSphere()
{
  gluDeleteQuadric(quadric);
}

void NonhierSphere::render() {
  glPushMatrix();
  glTranslated(m_pos[0], m_pos[1], m_pos[2]);
  gluQuadricNormals(quadric, GLU_SMOOTH);
  gluSphere(quadric, m_radius, 32, 32);
  glPopMatrix();
}

RayHit* NonhierSphere::raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces)
{
  Vector3D diff = m_pos - from;
  // if the sphere's center is behind this point, ignore it as it can only give the wrong normal
  if (diff.dot(ray) < - 0.01) {
    return NULL;
  }
  Vector3D A = diff.proj(ray);
  Vector3D B = diff - A;
  double b2 = B.length2();
  double r2 = m_radius * m_radius;
  /*
  if (sqrt(b2) - m_radius < 500) {
    std::cerr << "Ray to Sphere: " << sqrt(b2) << " Radius: " << m_radius << "\n";
    std::cerr << "Raycast with sphere.\n"
      << "Diff: " << diff << "Ray: " << ray << "\n  Diff proj Ray: " << A << "\n";
  }
  */
  if (b2 < r2) {
    double a = A.length();
    double c = sqrt(r2 - b2);
    double dist = a - c;
    if (dist <= 0.01 && backfaces) {
        dist = a + c;
    }
    //std::cerr << "Hit sphere, distance: " << dist << ".\n";
    if (dist > 0)
    {
      Point3D pt = from + ray.scaleTo(dist);
      Vector3D norm = pt - m_pos;
      // normalize normals later
      RayHit* result = new RayHit(dist, pt, m_material, norm);
      return result;
    } else {
        std::cout << "Diff: " << diff << " Ray: " << ray << " A: " << A << " a: " << a << " c: " << c <<" b2: " << b2 << " r2: " << r2 <<"\n";
        std::cout << "The sphere is behind this point\nDist: " << dist << "\n";
    }
  }
  return NULL;
}

void NonhierBox::render() {
  double x = m_pos[0];
  double y = m_pos[1];
  double z = m_pos[2];
  double sizex = m_size[0];
  double sizey = m_size[1];
  double sizez = m_size[2];
  std::cout << "Object is at " << m_pos << "and size: " << m_size << "\n";

  glBegin(GL_QUADS);
  // x and y
  glNormal3f(0.0f,0.0f,-1.0f);
  glVertex3d(x, y + sizey, z);
  glVertex3d(x + sizex, y + sizey, z);
  glVertex3d(x + sizex, y, z);
  glVertex3d(x, y, z);

  glNormal3f(0.0f,0.0f,1.0f);
  glVertex3d(x, y, z + sizez);
  glVertex3d(x + sizex, y, z + sizez);
  glVertex3d(x + sizex, y + sizey, z + sizez);
  glVertex3d(x, y + sizey, z + sizez);

  // y and z
  glNormal3f(-1.0f,0.0f,0.0f);
  glVertex3d(x, y, z + sizez);
  glVertex3d(x, y + sizey, z + sizez);
  glVertex3d(x, y + sizey, z);
  glVertex3d(x, y, z);

  glNormal3f(1.0f,0.0f,0.0f);
  glVertex3d(x + sizex, y, z);
  glVertex3d(x + sizex, y + sizey, z);
  glVertex3d(x + sizex, y + sizey, z + sizez);
  glVertex3d(x + sizex, y, z + sizez);

  // x and z
  glNormal3f(0.0f,-1.0f,0.0f);
  glVertex3d(x, y, z);
  glVertex3d(x + sizex, y, z);
  glVertex3d(x + sizex, y, z + sizez);
  glVertex3d(x, y, z + sizez);

  glNormal3f(0.0f,1.0f,0.0f);
  glVertex3d(x, y + sizey, z + sizez);
  glVertex3d(x + sizex, y + sizey, z + sizez);
  glVertex3d(x + sizex, y + sizey, z);
  glVertex3d(x, y + sizey, z);

  glEnd();
}

RayHit* NonhierBox::raycast(Point3D from, Vector3D ray, Material* m_material, bool backfaces)
{
    double tminx, tminy, tminz, tmaxx, tmaxy, tmaxz;
    // want to find min/max t values for the portion of the ray inside the cube along each axis
    // apparently negative 0 is a value and this will fail for it, but w/e
    // c division by 0 gives an infinite value, that should give correct results for < > min and max
    if (ray[0] >= 0) {
        tminx = (m_pos[0] - from[0])/ray[0];
        tmaxx = (m_pos[0] + m_size[0] - from[0])/ray[0];
    } else {
        tminx = (m_pos[0] + m_size[0] - from[0])/ray[0];
        tmaxx = (m_pos[0] - from[0])/ray[0];
    }
    if (ray[1] >= 0) {
        tminy = (m_pos[1] - from[1])/ray[1];
        tmaxy = (m_pos[1] + m_size[1] - from[1])/ray[1];
    } else {
        tminy = (m_pos[1] + m_size[1] - from[1])/ray[1];
        tmaxy = (m_pos[1] - from[1])/ray[1];
    }
    if (ray[2] >= 0) {
        tminz = (m_pos[2] - from[2])/ray[2];
        tmaxz = (m_pos[2] + m_size[2] - from[2])/ray[2];
    } else {
        tminz = (m_pos[2] + m_size[2] - from[2])/ray[2];
        tmaxz = (m_pos[2] - from[2])/ray[2];
    }
    double tmin = std::max(tminx, std::max(tminy, tminz));
    double tmax = std::min(tmaxx, std::min(tmaxy, tmaxz));
    if (tmin < 0 || tmin > tmax) {
        // tmin < 0 means the view is inside the cube
        // tmin > tmax means that the ray will miss the cube
        return NULL;
    } else {
        Point3D pt = from + tmin * ray;
        Vector3D norm; // the normal doesn't have to be normalized yet
        if (tmin == tminx) {
            norm = Vector3D(-ray[0], 0, 0);
        } else if (tmin == tminy) {
            norm = Vector3D(0, -ray[1], 0);
        } else if (tmin == tminz) {
            norm = Vector3D(0, 0, -ray[2]);
        } else {
            // this should not be possible
            std::cerr << "!!!!! A cube normal is bad\n";
            norm = pt - Point3D(0,0,0);
        }
        // dist is not relevant here since it will be determined again after inverse transformations have happened
        return new RayHit((pt - from).length(), pt, m_material, norm);
    }
}

NonhierBox::~NonhierBox()
{
}
