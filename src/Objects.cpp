#include "macros.h"
#include "raytracer.h"
#include "Objects.h"
#include "vector3d.h"

Vector3D Object::sampleRay(UniformRNGInfo &rngInfo) {
  float phi = rngInfo.distribution(rngInfo.rng) * 2 * M_PI;
  float costheta = (rngInfo.distribution(rngInfo.rng) - 0.5) * 2;
  float u = rngInfo.distribution(rngInfo.rng);
  float theta = acos(costheta);
  float r = cbrt(u);
  return Vector3D(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
}

Vector3D Bulb::getLightDirection(const Vector3D& point) const {
  return center - point;
}

Sphere::Sphere(float x1, float y1, float z1, float r1) : center(x1, y1, z1), r(r1) {
  aabbMin.x = x1 - r1;
  aabbMin.y = y1 - r1;
  aabbMin.z = z1 - r1;
  aabbMax.x = x1 + r1;
  aabbMax.y = y1 + r1;
  aabbMax.z = z1 + r1;
  centroid.x = x1;
  centroid.y = y1;
  centroid.z = z1;
}

IntersectionInfo Sphere::intersect(const Vector3D& origin, const Vector3D& direction) {
  float radiusSquared = r * r;
  Vector3D distanceFromSphere = center - origin;
  bool isInsideSphere = dot(distanceFromSphere, distanceFromSphere) < radiusSquared;

  Vector3D normalizedDirection = normalized(direction);
  float tc = dot(distanceFromSphere, normalizedDirection);
  
  if (isInsideSphere == false && tc < 0) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D d = origin + tc * normalizedDirection - center;
  float distanceSquared = dot(d, d);

  if (isInsideSphere == false && radiusSquared < distanceSquared) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  float tOffset = sqrt(radiusSquared - distanceSquared);
  float t = isInsideSphere ? tc + tOffset : tc - tOffset;

  Vector3D intersectionPoint = t * normalizedDirection + origin;
  return { t, intersectionPoint, normalized(intersectionPoint - center), this };
}

Plane::Plane(float A, float B, float C, float D) : normal(normalized(Vector3D(A, B, C))) {
  if (A != 0) {
    point.x = -D/A;
  } else if (B != 0) {
    point.y = -D/B;
  } else {
    point.z = -D/C;
  }
}

IntersectionInfo Plane::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D normalizedDirection = normalized(direction);

  float t = dot((point - origin), normal) / dot(normalizedDirection, normal);

  return (t < 0)
  ? IntersectionInfo{ INF_D, Vector3D(), Vector3D(), nullptr }
  : IntersectionInfo{ t, t * normalizedDirection + origin, normal, this };
}

Triangle::Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3)
  : p1(p1) {
  aabbMin.x = std::min(std::min(p1.x, p2.x), p3.x);
  aabbMin.y = std::min(std::min(p1.y, p2.y), p3.y);
  aabbMin.z = std::min(std::min(p1.z, p2.z), p3.z);
  aabbMax.x = std::max(std::max(p1.x, p2.x), p3.x);
  aabbMax.y = std::max(std::max(p1.y, p2.y), p3.y);
  aabbMax.z = std::max(std::max(p1.z, p2.z), p3.z);
  centroid.x = (p1.x + p2.x + p3.x) * ONE_THIRD;
  centroid.y = (p1.y + p2.y + p3.y) * ONE_THIRD;
  centroid.z = (p1.z + p2.z + p3.z) * ONE_THIRD;

  Vector3D p3p1Diff = p3 - p1;
  Vector3D p2p1Diff = p2 - p1;

  normal = normalized(cross(p2p1Diff, p3p1Diff));
  if (determinant(p1, p2, p3) > 0) {
    normal *= -1.0f;
  }


  Vector3D a1 = cross(p3p1Diff, normal);
  Vector3D a2 = cross(p2p1Diff, normal);

  e1 = 1.0 / dot(a1, p2p1Diff) * a1;
  e2 = 1.0 / dot(a2, p3p1Diff) * a2;
}

IntersectionInfo Triangle::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D normalizedDirection = normalized(direction);

  float t = dot((p1 - origin), normal) / dot(normalizedDirection, normal);

  if (t < 0) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D intersectionPoint = t * normalizedDirection + origin;

  float b2 = dot(e1, intersectionPoint - p1);
  float b3 = dot(e2, intersectionPoint - p1);
  float b1 = 1.0f - b3 - b2;

  return (b1 < 0 || b1 > 1 || b2 < 0 || b2 > 1 || b3 < 0 || b3 > 1)
  ? IntersectionInfo{ INF_D, Vector3D(), Vector3D(), nullptr }
  : IntersectionInfo{ t, intersectionPoint, normal, this };
}
