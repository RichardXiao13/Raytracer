#pragma once 

#include <vector>

#include "raytracer.h"
#include "vector3d.h"

using namespace std;

class Object;
struct IntersectionInfo;

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
  BVH(vector<Object*> &objects);
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);

private:
  void updateNodeBounds(Node *node);
  void partition(Node *node);
  double intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax);
  Node *root;
  vector<Object*> &objects;
};