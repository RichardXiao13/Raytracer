#pragma once 

#include <vector>
#include <memory>

#include "raytracer.h"
#include "vector3d.h"

using namespace std;

class Object;
struct IntersectionInfo;

class Box {
public:
  Box(const Vector3D& minPoint, const Vector3D& maxPoint) : minPoint_(minPoint), maxPoint_(maxPoint) {};
  void shrink(const Vector3D& minPoint);
  void expand(const Vector3D& maxPoint);
  double surfaceArea();

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
  BVH(vector<unique_ptr<Object>> &objects);
  ~BVH();
  IntersectionInfo findClosestObject(const Vector3D& origin, const Vector3D& direction);
  bool findAnyObject(const Vector3D& origin, const Vector3D& direction);
  int height();

private:
  void updateNodeBounds(Node *node);
  void partition(Node *node);
  double intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax);
  int height(Node *node);
  double calculateSAH(Node *node, int axis, double position);
  void recursiveDestructor(Node *node);
  Node *root;
  vector<unique_ptr<Object>> &objects;
};