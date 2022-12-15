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
#include "timer.h"

using namespace std;
using std::cout;

double getRayScaleX(double x, int w, int h) {
  return (2 * x - w) / max(w, h);
}
double getRayScaleY(double y, int w, int h) {
  return (h - 2 * y) / max(w, h);
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

RGBAColor Scene::illuminate(const IntersectionInfo& info, int giDepth) {
  const RGBAColor& objectColor = info.obj->color();
  const Vector3D& surfaceNormal = info.normal;
  const Vector3D& intersectionPoint = info.point;
  double newR = 0;
  double newG = 0;
  double newB = 0;
  
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->direction());
    if (pointInShadow(intersectionPoint + bias_ * surfaceNormal, normalizedLightDirection)) {
      continue;
    }

    double reflectance = fabs(dot(surfaceNormal, normalizedLightDirection));

    newR += (*it)->color().r * reflectance;
    newG += (*it)->color().g * reflectance;
    newB += (*it)->color().b * reflectance;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->getLightDirection(intersectionPoint));
    if (pointInShadow(intersectionPoint + bias_ * surfaceNormal, *it)) {
      continue;
    }
    
    double distance = magnitude((*it)->center() - intersectionPoint);
    double reflectance = fabs(dot(surfaceNormal, normalizedLightDirection)) / (distance * distance);

    newR += (*it)->color().r * reflectance;
    newG += (*it)->color().g * reflectance;
    newB += (*it)->color().b * reflectance;
  }

  // if (giDepth < globalIllumination) {
  //   double phi = (uniformDistribution(rng) + 0.5) * 2 * M_PI;
  //   double costheta = uniformDistribution(rng) * 2;
  //   double u = uniformDistribution(rng) + 0.5;
  //   double R = uniformDistribution(rng) + 0.5;
  //   double theta = acos(costheta);
  //   double r = R * cbrt(u);
  //   Vector3D sampledRay(r * sin(theta) * cos(phi), r * sin(theta) * sin(phi), r * cos(theta));
  //   Vector3D globalIlluminationDirection = surfaceNormal + sampledRay;
  //   RGBAColor giColor = raytrace(intersectionPoint + bias_ * surfaceNormal, globalIlluminationDirection, 0, giDepth + 1);
  //   giColor = giColor.a * giColor;
  //   newR += giColor.r;
  //   newG += giColor.g;
  //   newB += giColor.b;
  // }

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

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, int depth, int giDepth) {
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
    RGBAColor color = diffuse * illuminate(intersectInfo, giDepth);

    if (depth >= maxBounces) {
      return color;
    }

    if (transparency[0] > 0 || transparency[1] > 0 || transparency[2] > 0) {
      Vector3D normalizedSurfaceNormal = intersectInfo.normal;
      double ior = intersectInfo.obj->indexOfRefraction();
      double enteringCosine = dot(normalizedSurfaceNormal, normalizedDirection);

      // cosine is positive when vectors are in the same direction
      // so normal and incident ray are in the same direction
      // make normal point inside the object
      if (enteringCosine > 0) {
        normalizedSurfaceNormal = -1 * normalizedSurfaceNormal;
      } else {
        ior = 1.0 / ior;
        enteringCosine = -enteringCosine;
      }
      
      double k = 1.0 - ior * ior * (1.0 - enteringCosine * enteringCosine);

      if (k >= 0) {
        Vector3D refractedDirection = ior * normalizedDirection + (ior * enteringCosine - sqrt(k)) * normalizedSurfaceNormal;
        RGBAColor refractedColor = refraction * raytrace(intersectInfo.point - bias_ * normalizedSurfaceNormal, refractedDirection, depth + 1, giDepth);
        color += refractedColor.a * refractedColor;
      } else {
        Vector3D reflectedDirection = normalizedDirection - 2 * dot(normalizedSurfaceNormal, normalizedDirection) * normalizedSurfaceNormal;
        RGBAColor reflectedColor = refraction * raytrace(intersectInfo.point + bias_ * normalizedSurfaceNormal, reflectedDirection, depth + 1, giDepth);
        color += reflectedColor.a * reflectedColor;
      }
    }
    if (reflectance[0] > 0 || reflectance[1] > 0 || reflectance[2] > 0) {
      Vector3D reflectedDirection = normalizedDirection - 2 * dot(intersectInfo.normal, normalizedDirection) * intersectInfo.normal;
      RGBAColor reflectedColor = reflectance * raytrace(intersectInfo.point + bias_ * intersectInfo.normal, reflectedDirection, depth + 1, giDepth);
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

void Scene::createBVH() {
  cout << "Creating BVH" << endl;
  auto start = std::chrono::system_clock::now();

  bvh = new BVH(objects);

  auto end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  time_t end_time = chrono::system_clock::to_time_t(end);
  cout << "BVH creation time: " << elapsed_seconds.count() << "s" << endl;
}

PNG *Scene::renderFisheye() {
  int totalPixels = height_ * width_;
  int finishedPixels = 0;

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
        double Sx = getRayScaleX(x + uniformDistribution(rng) * allowAntiAliasing, width_, height_);
        double Sy = getRayScaleY(y + uniformDistribution(rng) * allowAntiAliasing, width_, height_);

        Sx *= invForwardLength;
        Sy *= invForwardLength;
        double r_2 = Sx * Sx + Sy * Sy;
        if (r_2 > 1) {
          continue;
        }
        forward = sqrt(1 - r_2) * normalizedForward;

        RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0, 0));
        if (color.a != 0) {
          avgColor += color;
          ++hits;
        }
      }

      avgColor.r *= invNumRays;
      avgColor.g *= invNumRays;
      avgColor.b *= invNumRays;
      avgColor.a = hits * invNumRays;

      img->getPixel(y, x) = avgColor;

      ++finishedPixels;
      displayRenderProgress(static_cast<double>(finishedPixels) / totalPixels);
    }
  }

  expose(img);

  auto end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  time_t end_time = chrono::system_clock::to_time_t(end);
  cout << "\nRaytracing elapsed time: " << elapsed_seconds.count() << "s" << endl;
  return img;
}

PNG *Scene::renderDefault() {
  int totalPixels = height_ * width_;
  int finishedPixels = 0;

  int allowAntiAliasing = min(1, numRays - 1);

  PNG *img = new PNG(width_, height_);

  auto start = std::chrono::system_clock::now();
  double invNumRays = 1.0 / numRays;
  
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      RGBAColor avgColor(0, 0, 0, 1);
      int hits = 0;

      for (int i = 0; i < numRays; ++i) {
        double Sx = getRayScaleX(x + uniformDistribution(rng) * allowAntiAliasing, width_, height_);
        double Sy = getRayScaleY(y + uniformDistribution(rng) * allowAntiAliasing, width_, height_);

        RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0, 0));
        if (color.a != 0) {
          avgColor += color;
          ++hits;
        }
      }

      avgColor.r *= invNumRays;
      avgColor.g *= invNumRays;
      avgColor.b *= invNumRays;
      avgColor.a = hits * invNumRays;

      img->getPixel(y, x) = avgColor;

      ++finishedPixels;
      displayRenderProgress(static_cast<double>(finishedPixels) / totalPixels);
    }
  }

  expose(img);

  auto end = chrono::system_clock::now();
  chrono::duration<double> elapsed_seconds = end-start;
  time_t end_time = chrono::system_clock::to_time_t(end);
  cout << "\nRaytracing elapsed time: " << elapsed_seconds.count() << "s" << endl;
  return img;
}

PNG *Scene::render(int seed) {
  createBVH();
  rng.seed(seed);

  if (fisheye) {
    cout << "Fisheye enabled." << endl;
    return renderFisheye();
  } else {
    return render();
  }
}


void Scene::expose(PNG *img) {
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      if (exposure >= 0) {
        RGBAColor &pixel = img->getPixel(y, x);
        pixel.r = exponentialExposure(pixel.r, exposure);
        pixel.g = exponentialExposure(pixel.g, exposure);
        pixel.b = exponentialExposure(pixel.b, exposure);
      }
    }
  }
}


/**
 * threadTaskDefault - default worker function for threads. Runs same routine as render default.
*/
// void threadTaskDefault(PNG *img, double invNumRays, SafeQueue<pair<int, int>> &tasks, int seed) {
//   pair<int, int> task;
//   mt19937 rng;
//   rng.seed(seed);
//   uniform_real_distribution<> rayDistribution = uniform_real_distribution<>(-0.5, 0.5);

//   // hacky... but does the job
//   while ((task = tasks.dequeue()).first != -1) {
//     int x = task.first;
//     int y = task.second;

//     RGBAColor avgColor(0, 0, 0, 1);
//     int hits = 0;

//     for (int i = 0; i < numRays; ++i) {
//       double Sx = getRayScaleX(x + rayDistribution(rng) * allowAntiAliasing, width_, height_);
//       double Sy = getRayScaleY(y + rayDistribution(rng) * allowAntiAliasing, width_, height_);

//       RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0, 0));
//       if (color.a != 0) {
//         avgColor += color;
//         ++hits;
//       }
//     }

//     avgColor.r *= invNumRays;
//     avgColor.g *= invNumRays;
//     avgColor.b *= invNumRays;
//     avgColor.a = hits * invNumRays;

//     img->getPixel(y, x) = avgColor;
//   }
// }

void threadTaskFisheye(PNG *img, SafeQueue<pair<int, int>> &tasks) {

}