#pragma once

#include "macros.h"
#include "PNG.h"
#include "vector3d.h"
#include "BVH.h"
#include "Objects.h"
#include "SafeQueue.h"
#include "SafeProgressBar.h"
#include "math_utils.h"

// A ray has origin 'eye' and direction 'forward' + Sx * 'right' + Sy * 'up'
float getRayScaleX(float x, int w, int h);
float getRayScaleY(float y, int w, int h);

struct RenderTask {
  int x;
  int y;
};

class Scene {
public: 
  Scene(int w, int h, const std::string& file)
    : width_(w), height_(h), filename_(file) {};
  ~Scene();
  void addObject(std::unique_ptr<Object> obj);
  void addPlane(std::unique_ptr<Plane> plane);
  void addLight(Light *light);
  size_t getNumObjects();
  PNG *render(int numThreads=4, int seed=56);
  void setExposure(float value);
  void setMaxBounces(int d);
  void setFilename(const std::string& fname);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction) const;
  IntersectionInfo findAnyObject(const Vector3D& origin, const Vector3D& direction) const;

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  std::string filename() {
    return filename_;
  }

  void setEye(const Vector3D& e) {
    eye = e;
  }

  void setForward(const Vector3D& f) {
    forward = f;
    right = normalized(cross(forward, up));
    up = normalized(cross(right, forward));
  }

  void setUp(const Vector3D& u) {
    right = normalized(cross(forward, u));
    up = normalized(cross(right, forward));
  }

  void setNumRays(int n) {
    numRays = n;
  }

  void enableFisheye() {
    fisheye = true;
  }

  Vector3D getEye() {
    return eye;
  }

  void setGlobalIllumination(int gi) {
    globalIllumination = gi;
  }
  void setSpecularRays(int specular) {
    specularRays = specular;
  }

  void setFocus(float f) {
    focus_ = f;
  }

  void setLens(float l) {
    lens_ = l;
  }

  Vector3D eye{0, 0, 0}; // POINT; NOT NORMALIZED
  Vector3D forward{0, 0, -1}; // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
  Vector3D right{1, 0, 0}; // NORMALIZED
  Vector3D up{0, 1, 0}; // NORMALIZED

private:
  RGBAColor illuminate(const Vector3D &rayDirection, const IntersectionInfo& info, UniformDistribution &sampler);
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
  float bias_ = 1e-4;
  float exposure = -1.0;
  int maxBounces = 4;
  int numRays = 1;
  std::unique_ptr<BVH> bvh;
  bool fisheye = false;
  int globalIllumination = 0;
  int specularRays = 4;
  float focus_ = -1.0;
  float lens_ = 0;
};
