#pragma once

#include "Object.h"
#include "Camera.h"

#include "../macros.h"
#include "../image/PNG.h"
#include "../vector/vector3d.h"
#include "../acceleration/BVH.h"
#include "../acceleration/SafeQueue.h"
#include "../acceleration/SafeProgressBar.h"
#include "../bsdf/math_utils.h"

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
float getRayScaleX(float x, int w, int h);
float getRayScaleY(float y, int w, int h);

struct IntersectionInfo;
class Object;
class Triangle;
class Sphere;
class Plane;
class Light;
class DistantLight;
class PointLight;
class EnvironmentLight;
class BVH;

struct RenderTask {
  int x;
  int y;
};

struct SceneOptions {
  float bias       = 1e-4;
  float exposure   = -1;
  int   maxBounces = 4;
  int   numRays    = 1;
  bool  fisheye    = false;
  float focus      = -1;
  float lens       = 0;
};

class Scene {
public:
  Scene() {};
  Scene(int w, int h, const std::string& file)
    : width_(w), height_(h), filename_(file) {};
  ~Scene();
  PNG *render(int numThreads=4, int seed=56);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction) const;
  IntersectionInfo findAnyObject(const Vector3D& origin, const Vector3D& direction) const;

  void addObject(std::unique_ptr<Object> obj);

  void addPlane(std::unique_ptr<Plane> plane) {
    planes.push_back(std::move(plane));
  }

  void addLight(Light *light) {
    lights.push_back(light);
  }

  void setFilename(const std::string& fname) {
    filename_ = fname;
  }

  int width() const {
    return width_;
  }

  int height() const {
    return height_;
  }

  void setWidth(int width) {
    width_ = width;
  }

  void setHeight(int height) {
    height_ = height;
  }

  std::string filename() const {
    return filename_;
  }

  Vector3D worldCenter() const {
    return centroidSum / objects.size();
  }

  Camera camera;
  SceneOptions options;

private:
  RGBAColor illuminate(const Vector3D &direction, const IntersectionInfo& info);
  RGBAColor raytrace(const Vector3D& origin, const Vector3D& direction, UniformDistribution &sampler);
  void expose(PNG *img);
  /**
   * threadTaskDefault - default worker function for threads.
  */
  void threadTaskDefault(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  /**
   * threadTaskFisheye - fisheye render worker function for threads.
  */
  void threadTaskFisheye(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  void threadTaskDOF(PNG *img, SafeQueue<RenderTask> *tasks, SafeProgressBar *counter);
  PNG *render(std::function<void (Scene *, PNG *, SafeQueue<RenderTask> *, SafeProgressBar *)> worker, int numThreads);

  std::vector<std::unique_ptr<Object>> objects;
  std::vector<std::unique_ptr<Plane>> planes;
  std::vector<Light*> lights;
  
  int width_;
  int height_;
  std::string filename_;
  std::unique_ptr<BVH> bvh;
  Vector3D centroidSum;
};
