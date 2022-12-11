#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <float.h>

#include "lodepng.h"
#include "raytracer.h"

using namespace std;

double getRayScaleX(double x, int w, int h) {
  return (2 * x - w) / max(w, h);
}
double getRayScaleY(double y, int w, int h) {
  return (h - 2 * y) / max(w, h);
}

Vector3D Bulb::getLightDirection(const Vector3D& point) {
  return center_ - point;
}

IntersectionInfo Sphere::intersect(const Vector3D& origin, const Vector3D& direction) {
  double radiusSquared = r * r;
  Vector3D distanceFromSphere = center - origin;
  bool isInsideSphere = dot(distanceFromSphere, distanceFromSphere) < radiusSquared;

  Vector3D normalizedDirection = normalized(direction);
  double tc = dot(distanceFromSphere, normalizedDirection);
  
  if (isInsideSphere == false && tc < 0) {
    return { -1, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D d = origin + tc * normalizedDirection - center;
  double distanceSquared = dot(d, d);

  if (isInsideSphere == false && radiusSquared < distanceSquared) {
    return { -1, Vector3D(), Vector3D(), nullptr };
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

IntersectionInfo Plane::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D surfaceNormal(A, B, C);
  Vector3D point;
  if (A != 0) {
    point[0] = -D/A;
  } else if (B != 0) {
    point[1] = -D/B;
  } else {
    point[2] = -D/C;
  }
  Vector3D normalizedDirection = normalized(direction);

  double t = dot((point - origin), surfaceNormal) / dot(normalizedDirection, surfaceNormal);

  if (t < 0) {
    return { -1, Vector3D(), Vector3D(), nullptr };
  }
  return { t, t * normalizedDirection + origin, surfaceNormal, this };
}

void Scene::addObject(Object *obj) {
  objects.push_back(obj);
}

void Scene::addLight(Light *light) {
  lights.push_back(light);
}

void Scene::addBulb(Bulb *bulb) {
  bulbs.push_back(bulb);
}

size_t Scene::getNumObjects() {
  return objects.size();
}

Scene::~Scene() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    delete *it;
  }
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    delete *it;
  }
  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    delete *it;
  }
}

IntersectionInfo Scene::findClosestObject(const Vector3D& origin, const Vector3D& direction) {
  IntersectionInfo closestInfo;
  closestInfo.t = DBL_MAX;

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(origin, direction);
    if (info.t >= 0 && info.t < closestInfo.t) {
      closestInfo = info;
    }
  }
  return closestInfo;
}

RGBAColor Scene::illuminate(const IntersectionInfo& info) {
  const RGBAColor& objectColor = info.obj->color();
  const Vector3D& surfaceNormal = info.normal;
  const Vector3D& intersectionPoint = info.point;
  double newR = 0;
  double newG = 0;
  double newB = 0;
  Vector3D normalizedSurfaceNormal = normalized(surfaceNormal);
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    if (pointInShadow(intersectionPoint + bias_ * normalizedSurfaceNormal, (*it)->direction())) {
      continue;
    }
    Vector3D normalizedLightDirection = normalized((*it)->direction());
    double reflectance = dot(normalizedSurfaceNormal, normalizedLightDirection);

    newR += objectColor.r * (*it)->color().r * reflectance;
    newG += objectColor.g * (*it)->color().g * reflectance;
    newB += objectColor.b * (*it)->color().b * reflectance;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    if (pointInShadow(intersectionPoint + bias_ * normalized(surfaceNormal), (*it)->getLightDirection(intersectionPoint))) {
      // continue;
    }
    Vector3D normalizedLightDirection = normalized((*it)->getLightDirection(intersectionPoint));
    double distance = magnitude((*it)->center() - intersectionPoint);
    double reflectance = dot(normalizedSurfaceNormal, normalizedLightDirection) / (distance * distance);

    newR += objectColor.r * (*it)->color().r * reflectance;
    newG += objectColor.g * (*it)->color().g * reflectance;
    newB += objectColor.b * (*it)->color().b * reflectance;
  }

  return RGBAColor(newR, newG, newB, objectColor.a);
}

bool Scene::pointInShadow(const Vector3D& point, const Vector3D& lightDirection) {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    if ((*it)->intersect(point, lightDirection).obj != nullptr) {
      return true;
    }
  }
  return false;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction) {
  IntersectionInfo intersectInfo = findClosestObject(origin, direction);
  if (intersectInfo.obj != nullptr) {
    return illuminate(intersectInfo);
  }
  return RGBAColor(1, 1, 1, 0);
}

PNG *Scene::render(const Vector3D& eye, const Vector3D& forward, const Vector3D& right, const Vector3D& up) {
  PNG *img = new PNG(width_, height_);
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      double Sx = getRayScaleX(x, width_, height_);
      double Sy = getRayScaleY(y, width_, height_);

      img->getPixel(y, x) = raytrace(eye, forward + Sx * right + Sy * up);
    }
  }
  return img;
}
