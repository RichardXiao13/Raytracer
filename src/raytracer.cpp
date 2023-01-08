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
  int finishedPixels = 0;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);
  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);

      RGBAColor color = raytrace(eye, forward + Sx * right + Sy * up, sampler);
      if (hasNaN(color) == false && color.a != 0) {
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
  int finishedPixels = 0;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);

  float invForwardLength = 1.0 / magnitude(forward);
  Vector3D normalizedForward = normalized(forward);

  // Avoid race condiiton
  Vector3D forwardCopy = forward;

  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);

      Sx *= invForwardLength;
      Sy *= invForwardLength;
      float r_2 = Sx * Sx + Sy * Sy;
      if (r_2 > 1) {
        continue;
      }
      forwardCopy = sqrt(1 - r_2) * normalizedForward;

      RGBAColor color = raytrace(eye, forwardCopy + Sx * right + Sy * up, sampler);
      if (hasNaN(color) == false && color.a != 0) {
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
  int finishedPixels = 0;

  float invNumRays = 1.0 / numRays;
  int allowAntiAliasing = std::min(1, numRays - 1);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      Vector3D rayDirection = forward + Sx * right + Sy * up;
      Vector3D intersectionPoint = focus_ / magnitude(rayDirection) * rayDirection + eye;
      float weight = sampler() * 2 * M_PI;
      float r = sampler() - 0.5f;
      Vector3D origin = r * cos(weight) * lens_ / magnitude(right) * right + r * sin(weight) * lens_ / magnitude(up) * up + eye;
      rayDirection = intersectionPoint - origin;

      RGBAColor color = raytrace(origin, rayDirection, sampler);
      if (hasNaN(color) == false && color.a != 0) {
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

IntersectionInfo Scene::findAnyObject(const Vector3D& origin, const Vector3D& direction) const {
  IntersectionInfo closestInfo = bvh->findClosestObject(origin, direction);
  if (closestInfo.obj != nullptr)
    return closestInfo;

  for (auto it = planes.begin(); it != planes.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(origin, direction);
    if (info.t < closestInfo.t)
      return info;
  }
  return closestInfo;
}

IntersectionInfo Scene::findClosestObject(const Vector3D& origin, const Vector3D& direction) const {
  IntersectionInfo closestInfo = bvh->findClosestObject(origin, direction);

  for (auto it = planes.begin(); it != planes.end(); ++it) {
    IntersectionInfo info = (*it)->intersect(origin, direction);
    if (info.t < closestInfo.t)
      closestInfo = info;
  }
  return closestInfo;
}

RGBAColor Scene::illuminate(const IntersectionInfo& info) {
  RGBAColor L;

  for (auto it = lights.begin(); it != lights.end(); ++it) {
    if ((*it)->pointInShadow(info.point, this) == false) {
      L += (*it)->intensity(info.point, info.normal);
    }
  }

  return info.obj->color * L;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, UniformDistribution &sampler) {
  RGBAColor L(0,0,0,0);
  RGBAColor beta(1,1,1,1);
  // bool isSpecular = false;
  Vector3D rayOrigin = origin;
  Vector3D rayDirection = direction;

  for (int bounces = 0; bounces < maxBounces; ++bounces) {
    IntersectionInfo intersectInfo = findClosestObject(rayOrigin, rayDirection);
    if (intersectInfo.obj == nullptr)
      break;
    
    Vector3D point = intersectInfo.point;
    Vector3D wo = -normalized(rayDirection);
    Vector3D outNormal = faceForward(wo, intersectInfo.normal);
    intersectInfo.point += bias_ * outNormal;
    const std::shared_ptr<Material> &material = intersectInfo.obj->material;

    L += beta * illuminate(intersectInfo);
    // Add metallic object specular contribution
    if (material->type == MaterialType::Metal)
      beta *= intersectInfo.obj->color;
    
    float pdf = 0.0f;
    BDFType type{};
    Vector3D wi;
    float contribution = material->bsdf.sampleFunc(wo, &wi, intersectInfo.normal, sampler, &pdf, &type);
    if (pdf == 0 || contribution == 0)
      break;

    beta *= contribution * std::abs(dot(wi, intersectInfo.normal)) / pdf;
    bool exiting = dot(wi, outNormal) > 0;
    rayOrigin = point + outNormal * (exiting ? bias_ : -bias_);
    rayDirection = wi;
    // isSpecular = static_cast<bool>(type & BDFType::PERFECT_SPECULAR);
  }
  
  return L;
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
