#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <utility>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <ctime>
#include <thread>

#include "lodepng.h"
#include "raytracer.h"
#include "SafeQueue.h"
#include "timer.h"
#include "math_utils.h"
#include "SafeProgressBar.h"
#include "Profiler.h"
#include "macros.h"

void Scene::threadTaskDefault(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);
  std::uniform_real_distribution<float> rayDistribution = std::uniform_real_distribution<float>(-0.5, 0.5);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformRNGInfo rngInfo(rng, sampleDistribution);

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 1);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + rayDistribution(rng) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + rayDistribution(rng) * allowAntiAliasing, width_, height_);

      RGBAColor color = clipColor(raytrace(eye, forward + Sx * right + Sy * up, 0, 0, rngInfo));
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    avgColor *= invNumRays;
    avgColor.a = hits * invNumRays;

    img->getPixel(y, x) = avgColor;
    counter->increment();
  }
}

void Scene::threadTaskFisheye(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);

  float invForwardLength = 1.0 / magnitude(forward);
  Vector3D normalizedForward = normalized(forward);

  // Avoid race condiiton
  Vector3D forwardCopy = forward;

  std::uniform_real_distribution<float> rayDistribution = std::uniform_real_distribution<float>(-0.5, 0.5);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformRNGInfo rngInfo(rng, sampleDistribution);

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 1);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + rayDistribution(rng) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + rayDistribution(rng) * allowAntiAliasing, width_, height_);

      Sx *= invForwardLength;
      Sy *= invForwardLength;
      float r_2 = Sx * Sx + Sy * Sy;
      if (r_2 > 1) {
        continue;
      }
      forwardCopy = sqrt(1 - r_2) * normalizedForward;

      RGBAColor color = clipColor(raytrace(eye, forwardCopy + Sx * right + Sy * up, 0, 0, rngInfo));
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    avgColor *= invNumRays;
    avgColor.a = hits * invNumRays;

    img->getPixel(y, x) = avgColor;
    counter->increment();
  }
}

void Scene::threadTaskDOF(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);
  std::uniform_real_distribution<float> rayDistribution = std::uniform_real_distribution<float>(-0.5, 0.5);
  std::uniform_real_distribution<float> lensDistribution = std::uniform_real_distribution<float>(0, 2 * M_PI);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformRNGInfo rngInfo(rng, sampleDistribution);

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 1);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + rayDistribution(rng) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + rayDistribution(rng) * allowAntiAliasing, width_, height_);
      Vector3D rayDirection = forward + Sx * right + Sy * up;
      Vector3D intersectionPoint = focus_ / magnitude(rayDirection) * rayDirection + eye;
      float weight = lensDistribution(rng);
      float r = rayDistribution(rng) + 0.5;
      Vector3D origin = r * cos(weight) * lens_ / magnitude(right) * right + r * sin(weight) * lens_ / magnitude(up) * up + eye;
      rayDirection = intersectionPoint - origin;

      RGBAColor color = clipColor(raytrace(origin, rayDirection, 0, 0, rngInfo));
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    avgColor *= invNumRays;
    avgColor.a = hits * invNumRays;

    img->getPixel(y, x) = avgColor;
    counter->increment();
  }
}

float getRayScaleX(float x, int w, int h) {
  return (2 * x - w) / std::max(w, h);
}
float getRayScaleY(float y, int w, int h) {
  return (h - 2 * y) / std::max(w, h);
}

void Scene::addObject(std::unique_ptr<Object> obj) {
  objects.push_back(std::move(obj));
}

void Scene::addPlane(std::unique_ptr<Plane> plane) {
  planes.push_back(std::move(plane));
}

void Scene::addLight(std::unique_ptr<Light> light) {
  lights.push_back(std::move(light));
}

void Scene::addBulb(std::unique_ptr<Bulb> bulb) {
  bulbs.push_back(std::move(bulb));
}

size_t Scene::getNumObjects() {
  return objects.size();
}

void Scene::setFilename(const std::string& fname) {
  filename_ = fname;
}

void Scene::setExposure(float value) {
  exposure = value;
}

void Scene::setMaxBounces(int d) {
  maxBounces = d;
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

RGBAColor Scene::illuminate(const IntersectionInfo& info, int giDepth, UniformRNGInfo &rngInfo) {
  const RGBAColor& objectColor = info.obj->color();
  const Vector3D& surfaceNormal = info.normal;
  const Vector3D& intersectionPoint = info.point + bias_ * surfaceNormal;
  RGBAColor newColor(0, 0, 0, objectColor.a);
  
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->direction());
    if (pointInShadow(intersectionPoint, normalizedLightDirection)) {
      continue;
    }

    float intensity = std::max(0.0f, dot(surfaceNormal, normalizedLightDirection));
    newColor += (*it)->color() * intensity;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->getLightDirection(intersectionPoint));
    if (pointInShadow(intersectionPoint, *it)) {
      continue;
    }
    
    float distance = magnitude((*it)->center() - intersectionPoint);
    float intensity = std::max(0.0f, dot(surfaceNormal, normalizedLightDirection)) / (distance * distance);

    newColor += (*it)->color() * intensity;
  }

  if (giDepth < globalIllumination) {
    Vector3D globalIlluminationDirection = normalized(surfaceNormal + info.obj->sampleRay(rngInfo));
    float intensity = std::max(0.0f, dot(surfaceNormal, globalIlluminationDirection));
    RGBAColor giColor = raytrace(intersectionPoint, globalIlluminationDirection, 0, giDepth + 1, rngInfo);
    newColor += giColor * intensity;
  }

  return RGBAColor(objectColor.r * newColor.r, objectColor.g * newColor.g, objectColor.b * newColor.b, objectColor.a);
}

bool Scene::pointInShadow(const Vector3D& point, const Vector3D& lightDirection) {
  return bvh->findAnyObject(point, lightDirection);
}

bool Scene::pointInShadow(const Vector3D& point, const std::unique_ptr<Bulb>& bulb) {
  float intersectToBulbDist = magnitude(bulb->center() - point);
  IntersectionInfo info = bvh->findClosestObject(point, bulb->getLightDirection(point));
  float objectToIntersect = magnitude(point - info.point);
  return info.obj != nullptr && objectToIntersect < intersectToBulbDist;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, int depth, int giDepth, UniformRNGInfo &rngInfo) {
  if (depth >= maxBounces) return RGBAColor(0, 0, 0, 0);
  
  Vector3D normalizedDirection = normalized(direction);
  IntersectionInfo intersectInfo = findClosestObject(origin, normalizedDirection);
  
  if (intersectInfo.obj == nullptr) return RGBAColor(0, 0, 0, 0);

  float ior = intersectInfo.obj->indexOfRefraction();

  if (intersectInfo.obj->material()->roughness > 0) {
    for (int i = 0; i < 3; ++i) {
      intersectInfo.normal[i] += intersectInfo.obj->getPerturbation(rngInfo.rng);
    }
    intersectInfo.normal = normalized(intersectInfo.normal);
  }

  // Make normals point away from incident ray
  if (dot(intersectInfo.normal, direction) > 0) {
    intersectInfo.normal = -1 * intersectInfo.normal;
  } else {
    ior = 1 / ior;
  }
  
  Vector3D &reflectance = intersectInfo.obj->shine();
  Vector3D &transparency = intersectInfo.obj->transparency();
  Vector3D refraction = (1.0 - reflectance) * transparency;
  Vector3D diffuse = 1.0 - refraction - reflectance;
  RGBAColor color = diffuse * illuminate(intersectInfo, giDepth, rngInfo);

  if (!isZero(transparency)) {
    Vector3D normalizedSurfaceNormal = intersectInfo.normal;
    Vector3D point = intersectInfo.point;

    Vector3D refractedDirection = refract(normalizedDirection, normalizedSurfaceNormal, ior, point, bias_);
    RGBAColor refractedColor = refraction * raytrace(point, refractedDirection, depth + 1, giDepth, rngInfo);
    color += refractedColor;
  }
  if (!isZero(reflectance)) {
    Vector3D reflectedDirection = reflect(normalizedDirection, intersectInfo.normal);
    RGBAColor reflectedColor = reflectance * raytrace(intersectInfo.point + bias_ * intersectInfo.normal, reflectedDirection, depth + 1, giDepth, rngInfo);
    color += reflectedColor;
  }
  
  return color;
}

PNG *Scene::render(std::function<void (Scene *, PNG *, SafeQueue<RenderTask> *, SafeProgressBar *)> worker, int numThreads) {
  Profiler p(Funcs::Render);

  int totalPixels = height_ * width_;
  int update = std::max(4096.0, 0.01 * totalPixels);

  PNG *img = new PNG(width_, height_);
\
  SafeQueue<RenderTask> tasks;
  SafeProgressBar counter(70, totalPixels, update);

  std::vector<std::thread> threads;
  for (int i = 0; i < numThreads; ++i) {
    threads.emplace_back(worker, this, img, &tasks, &counter);
  }
  
  for (int y = 0; y < height_; ++y) {
    for (int x = 0; x < width_; ++x) {
      tasks.enqueue({ x, y });
    }
  }
  for (int i = 0; i < numThreads; ++i) {
    tasks.enqueue({ -1, -1 });
  }
  for (auto it = threads.begin(); it != threads.end(); ++it) {
    it->join();
  }
  expose(img);
  return img;
}

PNG *Scene::render(int numThreads, int seed) {
  bvh = std::make_unique<BVH>(objects, numThreads);

  if (fisheye) {
    std::cout << "Fisheye enabled." << std::endl;
    return render(&Scene::threadTaskFisheye, numThreads);
  } else if (focus_ > 0) {
    std::cout << "Depth of Field enabled." << std::endl;
    return render(&Scene::threadTaskDOF, numThreads);
  } else {
    std::cout << "Default render." << std::endl;
    return render(&Scene::threadTaskDefault, numThreads);
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
