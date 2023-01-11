#include "raytracer.h"

#include "../macros.h"

#include "../image/lodepng.h"
#include "../bsdf/math_utils.h"
#include "../acceleration/SafeQueue.h"
#include "../acceleration/SafeProgressBar.h"
#include "../acceleration/Profiler.h"

#define THRESHOLD 512

float getRayScaleX(float x, int w, int h) {
  return (2 * x - w) / std::max(w, h);
}

float getRayScaleY(float y, int w, int h) {
  return (h - 2 * y) / std::max(w, h);
}

Scene::~Scene() {
  for (auto it = lights.begin(); it != lights.end(); ++it) {
    delete *it;
  }
}

void Scene::threadTaskDefault(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter) {
  RenderTask task;
  int finishedPixels = 0;

  float invNumRays = 1.0 / options.numRays;
  int allowAntiAliasing = std::min(1, options.numRays - 1);
  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < options.numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);

      RGBAColor color = raytrace(camera.eye, camera.forward + Sx * camera.right + Sy * camera.up, sampler);
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

  float invNumRays = 1.0 / options.numRays;
  int allowAntiAliasing = std::min(1, options.numRays - 1);

  float invForwardLength = 1.0 / magnitude(camera.forward);
  Vector3D normalizedForward = normalized(camera.forward);

  // Avoid race condiiton
  Vector3D forwardCopy = camera.forward;

  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < options.numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);

      Sx *= invForwardLength;
      Sy *= invForwardLength;
      float r_2 = Sx * Sx + Sy * Sy;
      if (r_2 > 1) {
        continue;
      }
      forwardCopy = sqrt(1 - r_2) * normalizedForward;

      RGBAColor color = raytrace(camera.eye, forwardCopy + Sx * camera.right + Sy * camera.up, sampler);
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

  float invNumRays = 1.0 / options.numRays;
  int allowAntiAliasing = std::min(1, options.numRays - 1);
  std::uniform_real_distribution<float> sampleDistribution = std::uniform_real_distribution<float>(0, 1.0);
  UniformDistribution sampler(std::mt19937(), std::uniform_real_distribution<float>(0, 1.0));

  // hacky... but does the job
  while ((task = tasks->dequeue()).x != -1) {
    int x = task.x;
    int y = task.y;

    RGBAColor avgColor(0, 0, 0, 0);
    int hits = 0;

    for (int i = 0; i < options.numRays; ++i) {
      float Sx = getRayScaleX(x + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      float Sy = getRayScaleY(y + (sampler() - 0.5f) * allowAntiAliasing, width_, height_);
      Vector3D rayDirection = camera.forward + Sx * camera.right + Sy * camera.up;
      Vector3D intersectionPoint = options.focus / magnitude(rayDirection) * rayDirection + camera.eye;
      float weight = sampler() * 2 * M_PI;
      float r = sampler() - 0.5f;
      Vector3D origin = r * cos(weight) * options.lens / magnitude(camera.right) * camera.right
                      + r * sin(weight) * options.lens / magnitude(camera.up) * camera.up
                      + camera.eye;
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

RGBAColor Scene::illuminate(const IntersectionInfo& info, UniformDistribution &sampler) {
  RGBAColor L;

  for (auto it = lights.begin(); it != lights.end(); ++it) {
      L += (*it)->intensity(info.point, info.normal, this, sampler);
  }

  return info.obj->getColor(info.point) * L;
}

RGBAColor Scene::raytrace(const Vector3D& origin, const Vector3D& direction, UniformDistribution &sampler) {
  RGBAColor L(0,0,0,0);
  RGBAColor beta(1,1,1,1);
  // bool isSpecular = false;
  Vector3D rayOrigin = origin;
  Vector3D rayDirection = direction;

  for (int bounces = 0; bounces < options.maxBounces; ++bounces) {
    IntersectionInfo intersectInfo = findClosestObject(rayOrigin, rayDirection);
    if (intersectInfo.obj == nullptr) {
      // Add environment lighting on miss
      for (auto it = lights.begin(); it != lights.end(); ++it) {
        L += (*it)->emittedLight(rayDirection);
      }
      break;
    }
    
    Vector3D point = intersectInfo.point;
    Vector3D wo = -normalized(rayDirection);
    Vector3D outNormal = faceForward(wo, intersectInfo.normal);
    intersectInfo.point += options.bias * outNormal;
    const std::shared_ptr<Material> &material = intersectInfo.obj->material;

    L += beta * illuminate(intersectInfo, sampler);
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
    rayOrigin = point + outNormal * (exiting ? options.bias : -options.bias);
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

  if (options.exposure >= 0)
    expose(img);
  
  return img;
}

PNG *Scene::render(int numThreads, int seed) {
  bvh = std::make_unique<BVH>(objects, numThreads);

  if (options.fisheye) {
    std::cout << "Fisheye enabled." << std::endl;
    return render(&Scene::threadTaskFisheye, numThreads);
  } else if (options.focus > 0) {
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
        RGBAColor &pixel = img->getPixel(y, x);
        pixel.r = exponentialExposure(pixel.r, options.exposure);
        pixel.g = exponentialExposure(pixel.g, options.exposure);
        pixel.b = exponentialExposure(pixel.b, options.exposure);
    }
  }
}

void Scene::addObject(std::unique_ptr<Object> obj) {
  centroidSum += obj->centroid;
  objects.push_back(std::move(obj));
}