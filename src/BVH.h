#pragma once 

#include <vector>
#include <memory>

#include "raytracer.h"
#include "vector3d.h"
#include "SafeProgressBar.h"

#define INF_D std::numeric_limits<float>::infinity()

class Object;
struct IntersectionInfo;

struct PartitionInfo {
  int bestAxis = -1;
  float bestPosition = 0;
  float bestCost = INF_D;
};

class Box {
public:
  Box() : minPoint_(INF_D, INF_D, INF_D), maxPoint_(-INF_D, -INF_D, -INF_D) {};
  Box(const Vector3D& minPoint, const Vector3D& maxPoint) : minPoint_(minPoint), maxPoint_(maxPoint) {};
  void shrink(const Vector3D& minPoint);
  void expand(const Vector3D& maxPoint);
  float surfaceArea();
  Vector3D &minPoint() {
    return minPoint_;
  }
  Vector3D &maxPoint() {
    return maxPoint_;
  }

private:
  Vector3D minPoint_;
  Vector3D maxPoint_;
};

/**
 * BVH - Bounding Volume Hierarchy
 * Constructs an Axis-Aligned Bounding Volume Hierarchy
*/
class BVH {
private:
  struct Node {
    Vector3D aabbMin;
    Vector3D aabbMax;
    Node *left;
    Node *right;
    int start;
    int numObjects;

    bool isLeaf() const {
      return numObjects > 0;
    }
  };

public:
  BVH(std::vector<std::unique_ptr<Object>> &objects, int maxThreads=1);
  ~BVH();
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);
  bool findAnyObject(const Vector3D& origin, const Vector3D& direction);
  int height();

private:
  void updateNodeBounds(Node *node);
  void partition(Node *node);
  float intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax);
  int height(Node *node);
  float calculateSAH(Node *node, int axis, float position);
  void recursiveDestructor(Node *node);
  PartitionInfo parallelizeSAH(Node *node, int start, int end, int maxThreads);
  PartitionInfo threadPartitionTask(Node *node, int start, int end);
  PartitionInfo findBestBucketSplit(Node *node);
  Node *root;
  std::vector<std::unique_ptr<Object>> &objects;
  int maxThreads;
  ProgressBar progress;
};