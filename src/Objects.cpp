#include <limits>

#include "Objects.h"

#define ONE_THIRD 1.0 / 3.0
#define INF_D std::numeric_limits<double>::infinity()

Vector3D Object::sampleRay() {
  double phi = sampleDistribution(rng_) * 2 * M_PI;
  double costheta = (sampleDistribution(rng_) - 0.5) * 2;
  double u = sampleDistribution(rng_);
  double R = sampleDistribution(rng_);
  double theta = acos(costheta);
  double r = R * cbrt(u);
  return Vector3D(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
}

Vector3D Bulb::getLightDirection(const Vector3D& point) const {
  return center_ - point;
}

Sphere::Sphere(double x1, double y1, double z1, double r1) : center(x1, y1, z1), r(r1) {
  aabbMin_[0] = x1 - r1;
  aabbMin_[1] = y1 - r1;
  aabbMin_[2] = z1 - r1;
  aabbMax_[0] = x1 + r1;
  aabbMax_[1] = y1 + r1;
  aabbMax_[2] = z1 + r1;
  centroid_[0] = x1;
  centroid_[1] = y1;
  centroid_[2] = z1;
}

IntersectionInfo Sphere::intersect(const Vector3D& origin, const Vector3D& direction) {
  double radiusSquared = r * r;
  Vector3D distanceFromSphere = center - origin;
  bool isInsideSphere = dot(distanceFromSphere, distanceFromSphere) < radiusSquared;

  Vector3D normalizedDirection = normalized(direction);
  double tc = dot(distanceFromSphere, normalizedDirection);
  
  if (isInsideSphere == false && tc < 0) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D d = origin + tc * normalizedDirection - center;
  double distanceSquared = dot(d, d);

  if (isInsideSphere == false && radiusSquared < distanceSquared) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  double tOffset = sqrt(radiusSquared - distanceSquared);
  double t = 0;

  if (isInsideSphere) {
    t = tc + tOffset;
  } else {
    t = tc - tOffset;
  }

  Vector3D intersectionPoint = t * normalizedDirection + origin;
  return { t, intersectionPoint, intersectionPoint - center, this };
}

Plane::Plane(double A, double B, double C, double D) : normal(A, B, C) {
  if (A != 0) {
    point[0] = -D/A;
  } else if (B != 0) {
    point[1] = -D/B;
  } else {
    point[2] = -D/C;
  }
}

IntersectionInfo Plane::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D normalizedDirection = normalized(direction);

  double t = dot((point - origin), normal) / dot(normalizedDirection, normal);

  if (t < 0) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }
  return { t, t * normalizedDirection + origin, normal, this };
}

Triangle::Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3)
  : p1(p1), p2(p2), p3(p3) {
  aabbMin_[0] = std::min(std::min(p1[0], p2[0]), p3[0]);
  aabbMin_[1] = std::min(std::min(p1[1], p2[1]), p3[1]);
  aabbMin_[2] = std::min(std::min(p1[2], p2[2]), p3[2]);
  aabbMax_[0] = std::max(std::max(p1[0], p2[0]), p3[0]);
  aabbMax_[1] = std::max(std::max(p1[1], p2[1]), p3[1]);
  aabbMax_[2] = std::max(std::max(p1[2], p2[2]), p3[2]);
  centroid_[0] = (p1[0] + p2[0] + p3[0]) * ONE_THIRD;
  centroid_[1] = (p1[1] + p2[1] + p3[1]) * ONE_THIRD;
  centroid_[2] = (p1[2] + p2[2] + p3[2]) * ONE_THIRD;

  Vector3D p3p1Diff = p3 - p1;
  Vector3D p2p1Diff = p2 - p1;

  normal = cross(p2p1Diff, p3p1Diff);

  Vector3D a1 = cross(p3p1Diff, normal);
  Vector3D a2 = cross(p2p1Diff, normal);

  e1 = 1.0 / dot(a1, p2p1Diff) * a1;
  e2 = 1.0 / dot(a2, p3p1Diff) * a2;
}

IntersectionInfo Triangle::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D normalizedDirection = normalized(direction);

  double t = dot((p1 - origin), normal) / dot(normalizedDirection, normal);

  if (t < 0) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D intersectionPoint = t * normalizedDirection + origin;

  double b2 = dot(e1, intersectionPoint - p1);
  double b3 = dot(e2, intersectionPoint - p1);
  double b1 = 1.0 - b3 - b2;

  if (b1 < 0 || b1 > 1 || b2 < 0 || b2 > 1 || b3 < 0 || b3 > 1) {
    return { INF_D, Vector3D(), Vector3D(), nullptr };
  }

  return { t, intersectionPoint, normal, this };
}
