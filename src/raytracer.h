#pragma once

#include "macros.h"
#include "PNG.h"
#include "vector3d.h"
#include "BVH.h"
#include "Objects.h"
#include "SafeQueue.h"
#include "SafeProgressBar.h"
#include "math_utils.h"
#include "Camera.h"

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
float getRayScaleX(float x, int w, int h);
float getRayScaleY(float y, int w, int h);

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

  void addObject(std::unique_ptr<Object> obj) {
    objects.push_back(std::move(obj));
  }

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

  std::string filename() {
    return filename_;
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
};
