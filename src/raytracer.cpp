#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <limits>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>

#include "lodepng.h"
#include "raytracer.h"
#include "SafeQueue.h"

using namespace std;
using std::cout;

double getRayScaleX(double x, int w, int h) {
  return (2 * x - w) / max(w, h);
}
double getRayScaleY(double y, int w, int h) {
  return (h - 2 * y) / max(w, h);
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
  aabbMin_[0] = min(min(p1[0], p2[0]), p3[0]);
  aabbMin_[1] = min(min(p1[1], p2[1]), p3[1]);
  aabbMin_[2] = min(min(p1[2], p2[2]), p3[2]);
  aabbMax_[0] = max(max(p1[0], p2[0]), p3[0]);
  aabbMax_[1] = max(max(p1[1], p2[1]), p3[1]);
  aabbMax_[2] = max(max(p1[2], p2[2]), p3[2]);
  centroid_[0] = (p1[0] + p2[0] + p3[0]) / 3.0;
  centroid_[1] = (p1[1] + p2[1] + p3[1]) / 3.0;
  centroid_[2] = (p1[2] + p2[2] + p3[2]) / 3.0;

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

void Scene::addPlane(Plane *plane) {
  planes.push_back(plane);
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

void Scene::setMaxBounces(int d) {
  maxBounces = d;
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
  IntersectionInfo closestInfo = bvh->findClosestObject(origin, direction);

  for (auto it = planes.begin(); it != planes.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(origin, direction);
    if (info.t >= 0 && (closestInfo.obj == nullptr || info.t < closestInfo.t)) {
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
  
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    if (pointInShadow(intersectionPoint + bias_ * surfaceNormal, (*it)->direction())) {
      continue;
    }
    Vector3D normalizedLightDirection = normalized((*it)->direction());
    double reflectance = fabs(dot(surfaceNormal, normalizedLightDirection));

    // can factor out objectColor to optimize
    newR += (*it)->color().r * reflectance;
    newG += (*it)->color().g * reflectance;
    newB += (*it)->color().b * reflectance;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    if (pointInShadow(intersectionPoint + bias_ * normalized(surfaceNormal), *it)) {
      continue;
    }
    Vector3D normalizedLightDirection = normalized((*it)->getLightDirection(intersectionPoint));
    double distance = magnitude((*it)->center() - intersectionPoint);
    double reflectance = fabs(dot(surfaceNormal, normalizedLightDirection)) / (distance * distance);

    newR += (*it)->color().r * reflectance;
    newG += (*it)->color().g * reflectance;
    newB += (*it)->color().b * reflectance;
  }

  return RGBAColor(objectColor.r * newR, objectColor.g * newG, objectColor.b * newB, objectColor.a);
}

bool Scene::pointInShadow(const Vector3D& point, const Vector3D& lightDirection) {
  return bvh->findClosestObject(point, lightDirection).obj != nullptr;
}

bool Scene::pointInShadow(const Vector3D& point, const Bulb *bulb) {
  double intersectToBulbDist = magnitude(bulb->center() - point);
  IntersectionInfo info = bvh->findClosestObject(point, bulb->getLightDirection(point));
  double objectToBulbDist = magnitude(bulb->center() - info.point);
  return info.obj != nullptr && objectToBulbDist < intersectToBulbDist;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, int depth) {
  Vector3D normalizedDirection = normalized(direction);
  IntersectionInfo intersectInfo = findClosestObject(origin, direction);
  
  if (intersectInfo.obj != nullptr) {
    intersectInfo.normal = normalized(intersectInfo.normal);
    if (intersectInfo.obj->roughness() > 0) {
      for (int i = 0; i < 3; ++i) {
        intersectInfo.normal[i] += intersectInfo.obj->getPerturbation(rng);
      }
      intersectInfo.normal = normalized(intersectInfo.normal);
    }
    
    Vector3D &reflectance = intersectInfo.obj->shine();
    Vector3D &transparency = intersectInfo.obj->transparency();
    Vector3D refraction = (1.0 - reflectance) * transparency;
    Vector3D diffuse = 1.0 - refraction - reflectance;
    RGBAColor color = diffuse * illuminate(intersectInfo);

    if (depth >= maxBounces) {
      return color;
    }

    if (transparency[0] > 0 || transparency[1] > 0 || transparency[2] > 0) {
      Vector3D normalizedSurfaceNormal = intersectInfo.normal;
      double ior = intersectInfo.obj->indexOfRefraction();
      double enteringCosine = dot(normalizedSurfaceNormal, normalizedDirection);

      if (enteringCosine > 0) {
        normalizedSurfaceNormal = -1 * normalizedSurfaceNormal;
      } else {
        ior = 1.0 / ior;
        enteringCosine = -enteringCosine;
      }
      
      double k = 1.0 - ior * ior * (1.0 - enteringCosine * enteringCosine);

      if (k >= 0) {
        Vector3D refractedDirection = ior * normalizedDirection + (ior * enteringCosine - sqrt(k)) * normalizedSurfaceNormal;
        RGBAColor refractedColor = refraction * raytrace(intersectInfo.point - bias_ * normalizedSurfaceNormal, refractedDirection, depth + 1);
        color += refractedColor.a * refractedColor;
      } else {
        Vector3D reflectedDirection = normalizedDirection - 2 * enteringCosine * normalizedSurfaceNormal;
        RGBAColor reflectedColor = refraction * raytrace(intersectInfo.point + bias_ * normalizedSurfaceNormal, reflectedDirection, depth + 1);
        color += reflectedColor.a * reflectedColor;
      }
    }
    if (reflectance[0] > 0 || reflectance[1] > 0 || reflectance[2] > 0) {
      Vector3D reflectedDirection = normalizedDirection - 2 * dot(intersectInfo.normal, normalizedDirection) * intersectInfo.normal;
      RGBAColor reflectedColor = reflectance * raytrace(intersectInfo.point + bias_ * intersectInfo.normal, reflectedDirection, depth + 1);
      color += reflectedColor.a * reflectedColor;
    }
    return color;
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

PNG *Scene::render(int seed) {
  bvh = new BVH(objects);

  int totalPixels = height_ * width_;
  int finishedPixels = 0;

  rng.seed(seed);

  int allowAntiAliasing = min(1, numRays - 1);

  PNG *img = new PNG(width_, height_);

  auto start = std::chrono::system_clock::now();
  double invNumRays = 1.0 / numRays;
  double invForwardLength = 1.0 / magnitude(forward);
  Vector3D normalizedForward = normalized(forward);
  
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      RGBAColor avgColor(0, 0, 0, 1);
      int hits = 0;

      for (int i = 0; i < numRays; ++i) {
        double Sx = getRayScaleX(x + aliasingDistribution(rng) * allowAntiAliasing, width_, height_);
        double Sy = getRayScaleY(y + aliasingDistribution(rng) * allowAntiAliasing, width_, height_);

        if (fisheye) {
          Sx *= invForwardLength;
          Sy *= invForwardLength;
          double r_2 = Sx * Sx + Sy * Sy;
          if (r_2 > 1) {
            continue;
          }
          forward = sqrt(1 - r_2) * normalizedForward;
        }

        RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0));
        if (color.a != 0) {
          avgColor += color;
          ++hits;
        }
      }

      avgColor.r *= invNumRays;
      avgColor.g *= invNumRays;
      avgColor.b *= invNumRays;
      avgColor.a = hits * invNumRays;

      if (exposure >= 0) {
        avgColor.r = exponentialExposure(avgColor.r, exposure);
        avgColor.g = exponentialExposure(avgColor.g, exposure);
        avgColor.b = exponentialExposure(avgColor.b, exposure);
      }

      img->getPixel(y, x) = avgColor;

      ++finishedPixels;
      displayRenderProgress(static_cast<double>(finishedPixels) / totalPixels);
    }
  }

  // Some computation here
  auto end = chrono::system_clock::now();

  chrono::duration<double> elapsed_seconds = end-start;
  time_t end_time = chrono::system_clock::to_time_t(end);

  cout << "\nElapsed time: " << elapsed_seconds.count() << "s" << endl;
  return img;
}
