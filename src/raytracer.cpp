#include "macros.h"
#include "lodepng.h"
#include "raytracer.h"
#include "SafeQueue.h"
#include "math_utils.h"
#include "SafeProgressBar.h"
#include "Profiler.h"

#define THRESHOLD 512

void Scene::threadTaskDefault(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;
  int finishedPixels = 0;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);
  std::uniform_real_distribution<float> rayDistribution = std::uniform_real_distribution<float>(-0.5, 0.5);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformRNGInfo rngInfo(rng, sampleDistribution);

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + rayDistribution(rng) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + rayDistribution(rng) * allowAntiAliasing, width_, height_);

      RGBAColor color = raytrace(eye, forward + Sx * right + Sy * up, 0, rngInfo);
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    if (hits != 0) {
      avgColor *= (1.0f/hits);
      avgColor.a = hits * invNumRays;
    }

    img->getPixel(y, x) = clipColor(avgColor);
    ++finishedPixels;
    if (finishedPixels % THRESHOLD == 0) {
      counter->increment(THRESHOLD);
    }
  }
  counter->increment(finishedPixels % THRESHOLD);
}

void Scene::threadTaskFisheye(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;
  int finishedPixels = 0;

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

    RGBAColor avgColor(0, 0, 0, 0);
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

      RGBAColor color = raytrace(eye, forwardCopy + Sx * right + Sy * up, 0, rngInfo);
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    if (hits != 0) {
      avgColor *= (1.0f/hits);
      avgColor.a = hits * invNumRays;
    }

    img->getPixel(y, x) = clipColor(avgColor);
    ++finishedPixels;
    if (finishedPixels % THRESHOLD == 0) {
      counter->increment(THRESHOLD);
    }
  }
  counter->increment(finishedPixels % THRESHOLD);
}

void Scene::threadTaskDOF(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  std::mt19937 rng;
  int finishedPixels = 0;

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

    RGBAColor avgColor(0, 0, 0, 0);
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

      RGBAColor color = raytrace(origin, rayDirection, 0, rngInfo);
      if (color.a != 0) {
        avgColor += color;
        ++hits;
      }
    }

    if (hits != 0) {
      avgColor *= (1.0f/hits);
      avgColor.a = hits * invNumRays;
    }

    img->getPixel(y, x) = clipColor(avgColor);
    ++finishedPixels;
    if (finishedPixels % THRESHOLD == 0) {
      counter->increment(THRESHOLD);
    }
  }
  counter->increment(finishedPixels % THRESHOLD);
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
    if (info.t < closestInfo.t) {
      closestInfo = info;
    }
  }
  return closestInfo;
}

RGBAColor Scene::illuminate(const Vector3D &rayDirection, const IntersectionInfo& info, int depth, UniformRNGInfo &rngInfo) {
  const RGBAColor& objectColor = info.obj->color;
  const Vector3D& surfaceNormal = info.normal;
  RGBAColor diffuse;
  RGBAColor specular;
  const std::unique_ptr<Material> &material = info.obj->material;
  const MaterialType type = material->type;
  const Vector3D outDirection = -1.0f * rayDirection;
  
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->direction);
    if (pointInShadow(info.point, normalizedLightDirection)) {
      continue;
    }

    float lambert = clipDot(surfaceNormal, normalizedLightDirection);
    diffuse += (*it)->color * lambert;
  }

  for (auto it = bulbs.begin(); it != bulbs.end(); ++it) {
    Vector3D normalizedLightDirection = normalized((*it)->getLightDirection(info.point));
    if (pointInShadow(info.point, *it)) {
      continue;
    }
    
    float distance = magnitude((*it)->center - info.point);
    float invDistance =  1.0f / (distance * distance);
    float lambert = clipDot(surfaceNormal, normalizedLightDirection);
    diffuse += (*it)->color * lambert * invDistance;
  }

  diffuse = M_1_PI * RGBAColor(objectColor.r * diffuse.r, objectColor.g * diffuse.g, objectColor.b * diffuse.b, objectColor.a);
  return diffuse;
}

bool Scene::pointInShadow(const Vector3D& point, const Vector3D& lightDirection) {
  return bvh->findAnyObject(point, lightDirection);
}

bool Scene::pointInShadow(const Vector3D& point, const std::unique_ptr<Bulb>& bulb) {
  float intersectToBulbDist = magnitude(bulb->center - point);
  IntersectionInfo info = bvh->findClosestObject(point, bulb->getLightDirection(point));
  float objectToIntersect = magnitude(point - info.point);
  // might be able to get rid of first condition since objectToIntersect is INF if obj doesn't exist
  return info.obj != nullptr && objectToIntersect < intersectToBulbDist;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, int depth, UniformRNGInfo &rngInfo) {
  if (depth >= maxBounces)
    return RGBAColor(0, 0, 0, 0);
  
  Vector3D wo = -normalized(direction);
  IntersectionInfo intersectInfo = findClosestObject(origin, direction);
  if (intersectInfo.obj == nullptr)
    return RGBAColor(0, 0, 0, 0);

  Vector3D point = intersectInfo.point;
  Vector3D outNormal = faceForward(wo, intersectInfo.normal);
  intersectInfo.point += bias_ * outNormal;

  BDF *perfectSpecular = intersectInfo.obj->material->specularBRDF;  
  RGBAColor color = illuminate(direction, intersectInfo, depth + 1, rngInfo);
  RGBAColor specular;
  if (perfectSpecular != nullptr) {
    int nSamples = 20;
    for (int i = 0; i < nSamples; ++i) {
      float pdf = 0.0f;
      Vector3D wi;
      float contribution = perfectSpecular->sampleFunc(wo, &wi, intersectInfo.normal, rngInfo, &pdf);
      bool exiting = dot(wi, outNormal) > 0;
      point += outNormal * (exiting ? bias_ : -bias_);
      if (pdf < 1e-8 || contribution < 1e-8)
        continue;
      RGBAColor Li = raytrace(point, wi, depth + 1, rngInfo);
      specular += contribution * Li / (pdf * nSamples);
    }
    return specular;
  }
  return color;
}

PNG *Scene::render(std::function<void (Scene *, PNG *, SafeQueue<RenderTask> *, SafeProgressBar *)> worker, int numThreads) {
  Profiler p(Funcs::Render);

  int totalPixels = height_ * width_;
  int update = std::max(4096.0, 0.01 * totalPixels);

  PNG *img = new PNG(width_, height_);

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
  for (size_t i = 0; i < threads.size(); ++i) {
    threads[i].join();
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
