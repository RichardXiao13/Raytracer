#pragma once

#include "../macros.h"

enum class Funcs {
  SceneConstruction,
  BVHConstruction,

  Render,
  Raytrace,
  Illumination,

  BVHIntersectClosest,
  BVHIntersectAny,
  BVHIntersectAABB,
  BVHIntersectInner
};

static const char * FuncNames[] = {
  "Scene construction",
  "BVH construction",
  "Rendering",
  "Raytracing",
  "Scene illumination",
  "BVH::findClosestObject",
  "BVH::findAnyObject",
  "BVH::IntersectAABB",
  "BVH::Intersect stack"
};

class Stats {
public:
  void updateRuntime(Funcs f, double timeDelta) {
    std::lock_guard<std::mutex> lock(m);
    runtimes[FuncNames[static_cast<int>(f)]] += timeDelta;
  }
  void print();

private:
  mutable std::mutex m;
  std::unordered_map<std::string, double> runtimes;
};

void printStats();

class Profiler {
public:
  Profiler(Funcs f) : f_(f), start(std::chrono::system_clock::now()) {};
  ~Profiler();
private:
  Funcs f_;
  std::chrono::system_clock::time_point start;
};
