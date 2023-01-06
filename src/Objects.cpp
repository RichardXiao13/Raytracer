#include "macros.h"
#include "raytracer.h"
#include "Objects.h"
#include "vector3d.h"

bool DistantLight::pointInShadow(const Vector3D &point, const Scene *scene) const {
  IntersectionInfo info = scene->findAnyObject(point, direction);
  return info.obj != nullptr;
}

RGBAColor DistantLight::intensity(const Vector3D &point, const Vector3D &n) const {
  return color * clipDot(n, direction);
}

bool Bulb::pointInShadow(const Vector3D &point, const Scene *scene) const {
  const Vector3D lightDirection = center - point;
  float intersectToBulbDist = magnitude(lightDirection);
  IntersectionInfo info = scene->findClosestObject(point, lightDirection);
  float objectToIntersect = magnitude(point - info.point);
  // might be able to get rid of first condition since objectToIntersect is INF if obj doesn't exist
  return info.obj != nullptr && objectToIntersect < intersectToBulbDist;
}

RGBAColor Bulb::intensity(const Vector3D &point, const Vector3D &n) const {
  const Vector3D lightDirection = center - point;
  float distance = magnitude(lightDirection);
  float invDistance =  1.0f / (distance * distance);
  return color * clipDot(n, lightDirection) * invDistance;
}

Sphere::Sphere(float x1, float y1, float z1, float r1, const RGBAColor &color, std::shared_ptr<Material> material)
  : Object(color, material), center(x1, y1, z1), r(r1) {
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

Plane::Plane(float A, float B, float C, float D, const RGBAColor &color, std::shared_ptr<Material> material)
  : Object(color, material), normal(normalized(Vector3D(A, B, C))) {
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

Triangle::Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3, const RGBAColor &color, std::shared_ptr<Material> material)
  : Object(color, material), p1(p1) {
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
  : IntersectionInfo{ t, intersectionPoint, normalized(n1 * b1 + n2 * b2 + n3 * b3), this };
}
