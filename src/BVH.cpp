#include <vector>
#include <limits>
#include <queue>
#include <functional>
#include <iostream>
#include <stack>
#include <memory>
#include <thread>
#include <future>

#include "raytracer.h"
#include "BVH.h"

#define INF_D numeric_limits<double>::infinity()
#define MIN_THREAD_WORK 64

using namespace std;


BVH::~BVH() {
  recursiveDestructor(root);
}

void BVH::recursiveDestructor(Node *node) {
  if (node->isLeaf()) {
    delete node;
    return;
  }
  recursiveDestructor(node->left);
  recursiveDestructor(node->right);
  delete node;
}

PartitionInfo BVH::parallelizeSAH(Node *node, int start, int end, int maxThreads) {
  int workPerThread = max((end - start) / maxThreads, MIN_THREAD_WORK);
  // Use futures as if they were threads. May not always be the case, but this is a nice abstraction
  vector<future<PartitionInfo>> threads;
  for (int i = start; i < end; i += workPerThread) {
    threads.emplace_back(async(&BVH::threadPartitionTask, this, node, i, min(i + workPerThread, end)));
  }
  PartitionInfo bestInfo;
  for (auto it = threads.begin(); it != threads.end(); ++it) {
    PartitionInfo info = it->get();
    if (info.bestCost < bestInfo.bestCost) {
      bestInfo = info;
    }
  }
  return bestInfo;
}

PartitionInfo BVH::threadPartitionTask(Node *node, int start, int end) {
  int bestAxis = -1;
  double bestPosition = 0;
  double bestCost = INF_D;

  for (int i = start; i < end; ++i) {
    Vector3D centroid = objects.at(i)->centroid();
    for (int axis = 0; axis < 3; ++axis) {
      double possiblePosition = centroid[axis];
      double cost = calculateSAH(node, axis, possiblePosition);
      if (cost < bestCost) {
        bestCost = cost;
        bestAxis = axis;
        bestPosition = possiblePosition;
      }
    }
  }
  
  return { bestAxis, bestPosition, bestCost };
}

void Box::shrink(const Vector3D& minPoint) {
  minPoint_[0] = min(minPoint_[0], minPoint[0]);
  minPoint_[1] = min(minPoint_[1], minPoint[1]);
  minPoint_[2] = min(minPoint_[2], minPoint[2]);
}

void Box::expand(const Vector3D& maxPoint) {
  maxPoint_[0] = max(maxPoint_[0], maxPoint[0]);
  maxPoint_[1] = max(maxPoint_[1], maxPoint[1]);
  maxPoint_[2] = max(maxPoint_[2], maxPoint[2]);
}

double Box::surfaceArea() {
  Vector3D extent = maxPoint_ - minPoint_;
  // Cost uses surface area of a box, but don't need to multiply by 2 since everything is multiplied by 2 in the heuristic
  // Can remove multiply if desired
  return 2 * (extent[0] * extent[1] + extent[1] * extent[2] + extent[0] * extent[2]);
}

BVH::BVH(vector<unique_ptr<Object>> &objects, int maxThreads) : objects(objects), maxThreads(maxThreads) {
  root = new Node();
  root->start = 0;
  root->numObjects = objects.size();
  updateNodeBounds(root);
  partition(root);
}

void BVH::updateNodeBounds(Node *node) {
  node->aabbMin = Vector3D(INF_D, INF_D, INF_D);
  node->aabbMax = Vector3D(-INF_D, -INF_D, -INF_D);
  int end = node->start + node->numObjects;
  for (int i = node->start; i < end; ++i) {
    Vector3D aabbObjMin = objects.at(i)->aabbMin();
    Vector3D aabbObjMax = objects.at(i)->aabbMax();

    node->aabbMin[0] = min(node->aabbMin[0], aabbObjMin[0]);
    node->aabbMin[1] = min(node->aabbMin[1], aabbObjMin[1]);
    node->aabbMin[2] = min(node->aabbMin[2], aabbObjMin[2]);

    node->aabbMax[0] = max(node->aabbMax[0], aabbObjMax[0]);
    node->aabbMax[1] = max(node->aabbMax[1], aabbObjMax[1]);
    node->aabbMax[2] = max(node->aabbMax[2], aabbObjMax[2]);
  }
}

double BVH::calculateSAH(Node *node, int axis, double position) {
  int leftBoxCount = 0;
  int rightBoxCount = 0;

  Box leftBox(Vector3D(INF_D, INF_D, INF_D), Vector3D(-INF_D, -INF_D, -INF_D));
  Box rightBox(Vector3D(INF_D, INF_D, INF_D), Vector3D(-INF_D, -INF_D, -INF_D));

  int end = node->start + node->numObjects;
  for(int i = node->start; i < end; ++i) {
    unique_ptr<Object> &obj = objects.at(i);
    if (obj->centroid()[axis] < position) {
      leftBox.shrink(obj->aabbMin());
      leftBox.expand(obj->aabbMax());
      leftBoxCount++;
    }
    else {
      rightBox.shrink(obj->aabbMin());
      rightBox.expand(obj->aabbMax());
      rightBoxCount++;
    }
  }

  double cost = leftBoxCount * leftBox.surfaceArea() + rightBoxCount * rightBox.surfaceArea();
  if (cost > 0) {
    return cost;
  }
  return INF_D;
}

void BVH::partition(Node *node) {
  // Base case: allow a max of 4 objects for a node before becoming a leaf
  if (node->numObjects <= 4) {
    return;
  }
  
  int bestAxis = -1;
  double bestPosition = 0;
  double bestCost = INF_D;
  int end = node->start + node->numObjects;

  PartitionInfo info = parallelizeSAH(node, node->start, end, maxThreads);

  Box parentBox(node->aabbMin, node->aabbMax);
  double parentCost = parentBox.surfaceArea() * node->numObjects;
  if (info.bestCost >= parentCost) {
    return;
  }

  int axis = info.bestAxis;
  double splitPosition = info.bestPosition;
  int i = node->start;
  int j = i + node->numObjects - 1;
  while (i <= j) {
    if (objects.at(i)->centroid()[axis] < splitPosition) {
      ++i;
    } else {
      swap(objects.at(i), objects.at(j));
      --j;
    }
  }

  // abort split if one of the sides is empty
  int leftNumObjects = i - node->start;
  if (leftNumObjects == 0 || leftNumObjects == node->numObjects) return;
  // create child nodes
  node->left = new Node();
  node->right = new Node();
  node->left->start = node->start;
  node->left->numObjects = leftNumObjects;
  node->right->start = i;
  node->right->numObjects = node->numObjects - leftNumObjects;
  node->numObjects = 0;
  updateNodeBounds(node->left);
  updateNodeBounds(node->right);

  partition(node->left);
  partition(node->right);
}

IntersectionInfo BVH::findClosestObject(const Vector3D& origin, const Vector3D& direction) {
  // Use vector as the underlying container
  // Better/faster for operations on the top of the stack, which is all this function does
  stack<pair<double, Node*>, vector<pair<double, Node*>>> to_visit;
  to_visit.push({ intersectAABB(origin, direction, root->aabbMin, root->aabbMax), root });

  double minDistance = INF_D;
  IntersectionInfo closestInfo{ INF_D, {}, {}, nullptr };

  while (to_visit.empty() == false) {
    pair<double, Node*> pairing = to_visit.top();
    double dist = pairing.first;
    Node *subtree = pairing.second;
    to_visit.pop();
    if (dist < minDistance) {
      if (subtree->isLeaf()) {
        int end = subtree->start + subtree->numObjects;
        for (int i = subtree->start; i < end; ++i) {
          IntersectionInfo info = objects.at(i)->intersect(origin, direction);
          if (info.t < minDistance) {
            minDistance = info.t;
            closestInfo = info;
          }
        }
      } else {
        double leftDistance = intersectAABB(origin, direction, subtree->left->aabbMin, subtree->left->aabbMax);
        double rightDistance = intersectAABB(origin, direction, subtree->right->aabbMin, subtree->right->aabbMax);
        if (leftDistance < rightDistance) {
          to_visit.push({ rightDistance, subtree->right });
          to_visit.push({ leftDistance, subtree->left });
        } else {
          to_visit.push({ leftDistance, subtree->left });
          to_visit.push({ rightDistance, subtree->right });
        }
      }
    }
  }

  return closestInfo;
}

bool BVH::findAnyObject(const Vector3D& origin, const Vector3D& direction) {
  stack<pair<double, Node*>, vector<pair<double, Node*>>> to_visit;
  to_visit.push({ intersectAABB(origin, direction, root->aabbMin, root->aabbMax), root });

  double minDistance = INF_D;

  while (to_visit.empty() == false) {
    pair<double, Node*> pairing = to_visit.top();
    double dist = pairing.first;
    Node *subtree = pairing.second;
    to_visit.pop();
    if (dist < minDistance) {
      if (subtree->isLeaf()) {
        int end = subtree->start + subtree->numObjects;
        for (int i = subtree->start; i < end; ++i) {
          IntersectionInfo info = objects.at(i)->intersect(origin, direction);
          if (info.obj != nullptr) {
            return true;
          }
        }
      } else {
        double leftDistance = intersectAABB(origin, direction, subtree->left->aabbMin, subtree->left->aabbMax);
        double rightDistance = intersectAABB(origin, direction, subtree->right->aabbMin, subtree->right->aabbMax);
        if (leftDistance < rightDistance) {
          to_visit.push({ rightDistance, subtree->right });
          to_visit.push({ leftDistance, subtree->left });
        } else {
          to_visit.push({ leftDistance, subtree->left });
          to_visit.push({ rightDistance, subtree->right });
        }
      }
    }
  }

  return false;
}

double BVH::intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax) {
  double tx_near = (aabbMin[0] - origin[0]) / direction[0];
  double tx_far = (aabbMax[0] - origin[0]) / direction[0];
  double tmin = min(tx_near, tx_far);
  double tmax = max(tx_near, tx_far);
  double ty_near = (aabbMin[1] - origin[1]) / direction[1];
  double ty_far = (aabbMax[1] - origin[1]) / direction[1];
  tmin = max(tmin, min(ty_near, ty_far));
  tmax = min(tmax, max(ty_near, ty_far));
  double tz_near = (aabbMin[2] - origin[2]) / direction[2];
  double tz_far = (aabbMax[2] - origin[2]) / direction[2];
  tmin = max(tmin, min(tz_near, tz_far));
  tmax = min(tmax, max(tz_near, tz_far));
  if (tmax >= tmin && tmax > 0) {
    return tmin;
  }
  return INF_D;
}

int BVH::height(Node *node) {
  if (node->isLeaf()) {
    return 1;
  }
  return max(height(node->left), height(node->right)) + 1;
}

int BVH::height() {
  return height(root);
}