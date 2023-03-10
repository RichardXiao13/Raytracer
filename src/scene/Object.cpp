#include "raytracer.h"
#include "Object.h"

#include "../macros.h"
#include "../vector/vector3d.h"

bool DistantLight::pointInShadow(const Vector3D &point, const Vector3D &direction, const Scene *scene) const {
  IntersectionInfo info = scene->findAnyObject(point, direction);
  return info.obj != nullptr;
}

RGBAColor DistantLight::intensity(const Vector3D &point, const Vector3D &n, const Scene *scene, UniformDistribution &sampler) const {
  if (pointInShadow(point, direction, scene))
    return RGBAColor(0, 0, 0, 1);
  return color * clipDot(n, direction);
}

bool PointLight::pointInShadow(const Vector3D &point, const Vector3D &direction, const Scene *scene) const {
  const Vector3D lightDirection = center - point;
  float intersectToBulbDist = magnitude(lightDirection);
  IntersectionInfo info = scene->findClosestObject(point, lightDirection);
  float objectToIntersect = magnitude(point - info.point);
  // might be able to get rid of first condition since objectToIntersect is INF if obj doesn't exist
  return info.obj != nullptr && objectToIntersect < intersectToBulbDist;
}

RGBAColor PointLight::intensity(const Vector3D &point, const Vector3D &n, const Scene *scene, UniformDistribution &sampler) const {
  const Vector3D lightDirection = center - point;
  if (pointInShadow(point, lightDirection, scene))
    return RGBAColor(0, 0, 0, 1);
  float distance = magnitude(lightDirection);
  float invDistance =  1.0f / (distance * distance);
  return color * clipDot(n, lightDirection) * invDistance;
}

bool EnvironmentLight::pointInShadow(const Vector3D &point, const Vector3D &direction, const Scene *scene) const {
  IntersectionInfo info = scene->findAnyObject(point, direction);
  return info.obj != nullptr;
}

RGBAColor EnvironmentLight::intensity(const Vector3D &point, const Vector3D &n, const Scene *scene, UniformDistribution &sampler) const {
  Vector3D wi = sampleHemisphere(n, sampler);
  if (pointInShadow(point, wi, scene))
    return RGBAColor(0, 0, 0, 1);
  return emittedLight(wi) * clipDot(n, wi);
}

RGBAColor EnvironmentLight::emittedLight(const Vector3D &direction) const {
  if (luminanceMap == nullptr)
    return color;
  const Vector3D mapCoordinates = sphericalToUV(center + direction * radius, luminanceMap);
  unsigned x = static_cast<unsigned>(mapCoordinates.x);
  unsigned y = static_cast<unsigned>(mapCoordinates.y);
  return luminanceMap->getPixel(y, x);
}

Sphere::Sphere(
  const Vector3D &center,
  float r,
  const RGBAColor &color,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> textureMap
)
  : Object(color, material, textureMap), center(center), r(r)
{
  aabbMin.x = center.x - r;
  aabbMin.y = center.y - r;
  aabbMin.z = center.z - r;
  aabbMax.x = center.x + r;
  aabbMax.y = center.y + r;
  aabbMax.z = center.z + r;
  centroid = center;
}

IntersectionInfo Sphere::intersect(const Vector3D& origin, const Vector3D& direction) const {
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

RGBAColor Sphere::getColor(const Vector3D &intersectionPoint) const {
  if (textureMap == nullptr)
    return color;
  
  const Vector3D textureCoordinates = sphericalToUV(intersectionPoint - center, textureMap);
  unsigned x = static_cast<unsigned>(textureCoordinates.x);
  unsigned y = static_cast<unsigned>(textureCoordinates.y);
  return textureMap->getPixel(y, x);
}

Plane::Plane(
  const Vector3D &normal,
  float D,
  const RGBAColor &color,
  std::shared_ptr<Material> material,
  const Vector3D &textureTopLeft,
  float textureZoom,
  const Vector3D &textureShift,
  std::shared_ptr<PNG> textureMap
)
  : Object(color, material, textureMap), normal(normalized(normal)),
    textureTopLeft(textureTopLeft), textureZoom(textureZoom), textureShift(textureShift)
{
  if (normal.x != 0) {
    point.x = -D/normal.x;
  } else if (normal.y != 0) {
    point.y = -D/normal.y;
  } else {
    point.z = -D/normal.z;
  }
}

IntersectionInfo Plane::intersect(const Vector3D& origin, const Vector3D& direction) const {
  Vector3D normalizedDirection = normalized(direction);

  float t = dot((point - origin), normal) / dot(normalizedDirection, normal);

  return (t < 0)
  ? IntersectionInfo{ INF_D, Vector3D(), Vector3D(), nullptr }
  : IntersectionInfo{ t, t * normalizedDirection + origin, normal, this };
}

RGBAColor Plane::getColor(const Vector3D &intersectionPoint) const {
  if (textureMap == nullptr)
    return color;

  Vector3D transformedPoint = transformToWorld(intersectionPoint.y, intersectionPoint.x, intersectionPoint.z, Vector3D(0, 0, 1));
  transformedPoint = transformedPoint / transformedPoint.z;
  float x = textureZoom * (transformedPoint.x - textureTopLeft.x) + textureShift.x;
  float y = textureZoom * (transformedPoint.y - textureTopLeft.y) + textureShift.y;
  float width = textureMap->width();
  float height = textureMap->height();
  x = x > 0 ? std::fmod(x, width) : width - std::fmod(-x, width);
  y = y > 0 ? std::fmod(y, height) : height - std::fmod(-y, height);
  x = clamp(x, 0, width - 1);
  y = clamp(height - y, 0, height - 1);
  return textureMap->getPixel(y, x);
}

Triangle::Triangle(
  const Vector3D& p1,
  const Vector3D& p2,
  const Vector3D& p3,
  const RGBAColor &color,
  std::shared_ptr<Material> material,
  const Vector3D &t1,
  const Vector3D &t2,
  const Vector3D &t3,
  std::shared_ptr<PNG> textureMap
)
  : Object(color, material, textureMap), p1(p1), t1(t1), t2(t2), t3(t3)
{
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

IntersectionInfo Triangle::intersect(const Vector3D& origin, const Vector3D& direction) const {
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

RGBAColor Triangle::getColor(const Vector3D &intersectionPoint) const {
  if (textureMap == nullptr)
    return color;
  
  float b2 = dot(e1, intersectionPoint - p1);
  float b3 = dot(e2, intersectionPoint - p1);
  float b1 = 1.0f - b3 - b2;
  Vector3D textureCoordinates = t1 * b1 + t2 * b2 + t3 * b3;
  return textureMap->getPixel(textureCoordinates.y, textureCoordinates.x);
}

void Triangle::setTextureCoordinates(const Vector3D &tex1, const Vector3D &tex2, const Vector3D &tex3) {
  t1 = tex1;
  t2 = tex2;
  t3 = tex3;
}
