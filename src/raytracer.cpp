#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <float.h>
#include <iostream>
#include <iomanip>

#include "lodepng.h"
#include "raytracer.h"

using namespace std;

double getRayScaleX(double x, int w, int h) {
  return (2 * x - w) / max(w, h);
}
double getRayScaleY(double y, int w, int h) {
  return (h - 2 * y) / max(w, h);
}

Vector3D Bulb::getLightDirection(const Vector3D& point) const {
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
    return { -1, Vector3D(), Vector3D(), nullptr };
  }
  return { t, t * normalizedDirection + origin, normal, this };
}

Triangle::Triangle(const Vector3D& p1, const Vector3D& p2, const Vector3D& p3)
  : p1(p1), p2(p2), p3(p3) {
  Vector3D p3p1Diff = p3 - p1;
  Vector3D p2p1Diff = p2 - p1;

  normal = cross(p2p1Diff, p3p1Diff);

  if (normal[2] < 0) {
    normal = -1 * normal;
  }

  Vector3D a1 = cross(p3p1Diff, normal);
  Vector3D a2 = cross(p2p1Diff, normal);

  e1 = 1.0 / dot(a1, p2p1Diff) * a1;
  e2 = 1.0 / dot(a2, p3p1Diff) * a2;
}

IntersectionInfo Triangle::intersect(const Vector3D& origin, const Vector3D& direction) {
  Vector3D normalizedDirection = normalized(direction);

  double t = dot((p1 - origin), normal) / dot(normalizedDirection, normal);

  if (t < 0) {
    return { -1, Vector3D(), Vector3D(), nullptr };
  }

  Vector3D intersectionPoint = t * normalizedDirection + origin;

  double b2 = dot(e1, intersectionPoint - p1);
  double b3 = dot(e2, intersectionPoint - p1);
  double b1 = 1.0 - b3 - b2;

  if (b1 < 0 || b1 > 1 || b2 < 0 || b2 > 1 || b3 < 0 || b3 > 1) {
    return { -1, Vector3D(), Vector3D(), nullptr };
  }

  return { t, intersectionPoint, normal, this };
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

void Scene::addPoint(double x, double y, double z) {
  points.push_back(Vector3D(x, y, z));
}

Vector3D &Scene::getPoint(int i) {
  if (i < 0) {
    i += points.size() + 1;
  }
  return points.at(i);
}

size_t Scene::getNumObjects() {
  return objects.size();
}

void Scene::setExposure(double value) {
  exposure = value;
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
  closestInfo.obj = nullptr;

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

    // can factor out objectColor to optimize
    newR += objectColor.r * (*it)->color().r * reflectance;
    newG += objectColor.g * (*it)->color().g * reflectance;
    newB += objectColor.b * (*it)->color().b * reflectance;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    if (pointInShadow(intersectionPoint + bias_ * normalized(surfaceNormal), *it)) {
      continue;
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
    IntersectionInfo info = (*it)->intersect(point, lightDirection);
    if (info.obj != nullptr) {
      return true;
    }
  }
  return false;
}

bool Scene::pointInShadow(const Vector3D& point, const Bulb *bulb) {
  double intersectToBulbDist = magnitude(bulb->center() - point);

  for (auto it = objects.begin(); it != objects.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(point, bulb->getLightDirection(point));
    double objectToBulbDist = magnitude(bulb->center() - info.point);
    if (info.obj != nullptr && objectToBulbDist < intersectToBulbDist) {
      return true;
    }
  }
  return false;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, int depth) {
  IntersectionInfo intersectInfo = findClosestObject(origin, direction);
  if (intersectInfo.obj != nullptr) {
    double shine = intersectInfo.obj->shine();
    if (depth < maxBounces && shine > 0) {
      Vector3D normalizedSurfaceNormal = normalized(intersectInfo.normal);
      Vector3D reflectedDirection = direction - 2 * dot(normalizedSurfaceNormal, direction) * normalizedSurfaceNormal;
      RGBAColor shinedColor = raytrace(intersectInfo.point + bias_ * normalized(intersectInfo.normal), reflectedDirection, depth + 1);
      return (1 - shine) * illuminate(intersectInfo) + shinedColor.a * shine * shinedColor;
    } else if (depth < maxBounces) {
      return illuminate(intersectInfo);
    }
  }
  return RGBAColor(1, 1, 1, 0);
}

void displayRenderProgress(double progress, int barWidth) {
  // https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
  int pos = barWidth * progress;
  cout << "[";

  for (int i = 0; i < barWidth; ++i) {
    if (i < pos) cout << "=";
    else if (i == pos) cout << ">";
    else cout << " ";
  }

  std::cout << "] " << fixed << setprecision(2) << progress * 100.0 << " % \r";
  std::cout.flush();
}

PNG *Scene::render(const Vector3D& eye, const Vector3D& forward, const Vector3D& right, const Vector3D& up) {
  int totalPixels = height_ * width_;
  int finishedPixels = 0;

  PNG *img = new PNG(width_, height_);
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      double Sx = getRayScaleX(x, width_, height_);
      double Sy = getRayScaleY(y, width_, height_);

      RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0));
      if (exposure >= 0) {
        color.r = exponentialExposure(color.r, exposure);
        color.g = exponentialExposure(color.g, exposure);
        color.b = exponentialExposure(color.b, exposure);
      }
      img->getPixel(y, x) = color;

      ++finishedPixels;
      displayRenderProgress(static_cast<double>(finishedPixels) / totalPixels);
    }
  }
  return img;
}
