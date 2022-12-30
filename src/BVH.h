#pragma once 

#include "macros.h"
#include "raytracer.h"
#include "vector3d.h"
#include "SafeProgressBar.h"

class Object;
struct IntersectionInfo;

struct PartitionInfo {
  int bestAxis = -1;
  float bestPosition = 0;
  float bestCost = INF_D;
};

class Box {
public:
  Box() : minPoint(INF_D, INF_D, INF_D), maxPoint(-INF_D, -INF_D, -INF_D) {};
  Box(const Vector3D& minPoint, const Vector3D& maxPoint) : minPoint(minPoint), maxPoint(maxPoint) {};
  void shrink(const Vector3D& minPoint);
  void expand(const Vector3D& maxPoint);
  inline float surfaceArea();

  Vector3D minPoint;
  Vector3D maxPoint;
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

  struct FlattenedNode {
    Vector3D aabbMin;
    Vector3D aabbMax;
    union {
      int right;
      int start;
    };
    int numObjects;

    inline bool isLeaf() const {
      return numObjects > 0;
    }
  };

public:
  BVH(std::vector<std::unique_ptr<Object>> &objects, int maxThreads=1);
  ~BVH();
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);
  bool findAnyObject(const Vector3D& origin, const Vector3D& direction);

private:
  void updateNodeBounds(Node *node);
  int partition(Node *node);
  float intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax);
  float calculateSAH(Node *node, int axis, float position);
  PartitionInfo parallelizeSAH(Node *node, int start, int end, int maxThreads);
  PartitionInfo threadPartitionTask(Node *node, int start, int end);
  PartitionInfo findBestBucketSplit(Node *node);
  void flatten(Node *node, int &idx);
  std::vector<std::unique_ptr<Object>> &objects;
  FlattenedNode *nodes;
  int maxThreads;
  SafeProgressBar progress;
};