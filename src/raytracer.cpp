#include "macros.h"
#include "lodepng.h"
#include "raytracer.h"
#include "SafeQueue.h"
#include "math_utils.h"
#include "SafeProgressBar.h"
#include "Profiler.h"

#define THRESHOLD 512

Scene::~Scene() {
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    delete *it;
  }
}

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

void Scene::addLight(Light *light) {
  lights.push_back(light);
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

IntersectionInfo Scene::findAnyObject(const Vector3D& origin, const Vector3D& direction) const {
  IntersectionInfo closestInfo = bvh->findClosestObject(origin, direction);
  if (closestInfo.obj != nullptr) {
    return closestInfo;
  }

  for (auto it = planes.begin(); it != planes.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(origin, direction);
    if (info.t < closestInfo.t) {
      return info;
    }
  }
  return closestInfo;
}

IntersectionInfo Scene::findClosestObject(const Vector3D& origin, const Vector3D& direction) const {
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
  const Vector3D wo = -rayDirection;
  RGBAColor Le;

  for (auto it = lights.begin(); it != lights.end(); ++it) {
    if ((*it)->pointInShadow(info.point, this) == false) {
      Le += (*it)->intensity(info.point, surfaceNormal);
    }
  }

  // Add global illumination contribution
  BDF *diffuseBSDF = info.obj->material->diffuseBRDF;
  float pdf = 0.0f;
  Vector3D wi;
  float contribution = diffuseBSDF->sampleFunc(wo, &wi, surfaceNormal, rngInfo, &pdf);
  if (pdf != 0 && contribution != 0) {
    RGBAColor Li = raytrace(info.point, wi, depth + 1, rngInfo);
    Le += contribution * Li / pdf;
  }

  return objectColor * Le;
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

  const std::unique_ptr<Material> &material = intersectInfo.obj->material;

  BDF *specularBSDF = material->specularBRDF;  
  RGBAColor diffuse;
  RGBAColor specular;

  if (material->Kd > 0)
    diffuse = material->Kd * illuminate(direction, intersectInfo, depth + 1, rngInfo);

  if (specularBSDF != nullptr && material->Ks > 0) {
    // Accumulate specular reflection/transmission
      float pdf = 0.0f;
      Vector3D wi;
      float contribution = specularBSDF->sampleFunc(wo, &wi, intersectInfo.normal, rngInfo, &pdf);
      bool exiting = dot(wi, outNormal) > 0;
      point += outNormal * (exiting ? bias_ : -bias_);
      if (pdf != 0 && contribution != 0) {
        RGBAColor Li = raytrace(point, wi, depth + 1, rngInfo);
        specular += contribution * Li / pdf;
        // Add metallic object specular contribution
        if (intersectInfo.obj->material->type == MaterialType::Metal)
          specular *= intersectInfo.obj->color;
      }

    specular = material->Ks * specular;
  }
  return diffuse + specular;
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
