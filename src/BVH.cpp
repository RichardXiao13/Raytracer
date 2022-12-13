#include <vector>
#include <limits>
#include <queue>
#include <functional>

#include "raytracer.h"
#include "BVH.h"

using namespace std;

BVH::BVH(vector<Object*> &objects) : objects(objects) {
  root = new Node();
  root->start = 0;
  root->numObjects = objects.size();
  updateNodeBounds(root);
  partition(root);
}

void BVH::updateNodeBounds(Node *node) {
  double inf = numeric_limits<double>::infinity();
  node->aabbMin = Vector3D(-inf, -inf, -inf);
  node->aabbMax = Vector3D(inf, inf, inf);
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

void BVH::partition(Node *node) {
  if (node->numObjects <= 2) {
    return;
  }

  Vector3D axisLengths = node->aabbMax - node->aabbMin;
  int axis = 0;
  if (axisLengths[1] > axisLengths[0]) {
    axis = 1;
  } else if (axisLengths[2] > axisLengths[axis]) {
    axis = 2;
  }

  double splitPosition = node->aabbMin[axis] + axisLengths[axis] * 0.5;
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
  auto cmp = [](const pair<double, Node*>& a, const pair<double, Node*>& b) {
    return a.first > b.first;
  };
  priority_queue<pair<double, Node*>, vector<pair<double, Node*>>, decltype(cmp)> min_queue(cmp);

  min_queue.push({ intersectAABB(origin, direction, root->aabbMin, root->aabbMax), root });
  double minDistance = numeric_limits<double>::infinity();
  IntersectionInfo closestInfo{ -1, {}, {}, nullptr };
  
  while (min_queue.empty() == false) {
    pair<double, Node*> pairing = min_queue.top();
    double dist = pairing.first;
    Node *subtree = pairing.second;
    min_queue.pop();
    if (dist >= 0 && dist < minDistance) {
      if (subtree->isLeaf()) {
        int end = subtree->start + subtree->numObjects;
        for (int i = subtree->start; i < end; ++i) {
          IntersectionInfo info = objects.at(i)->intersect(origin, direction);
          if (info.obj != nullptr && info.t < minDistance) {
            minDistance = info.t;
            closestInfo = info;
          }
        }
      } else {
        min_queue.push({ intersectAABB(origin, direction, subtree->left->aabbMin, subtree->left->aabbMax), subtree->left });
        min_queue.push({ intersectAABB(origin, direction, subtree->right->aabbMin, subtree->right->aabbMax), subtree->right });
      }
    }
  }

  return closestInfo;
}

double BVH::intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax) {
  double tx_near = (aabbMin[0] - origin[0]) / direction[0];
  double tx_far = (aabbMax[0] - origin[0]) / direction[0];
  double ty_near = (aabbMin[1] - origin[1]) / direction[1];
  double ty_far = (aabbMax[1] - origin[1]) / direction[1];
  double tz_near = (aabbMin[2] - origin[2]) / direction[2];
  double tz_far = (aabbMax[2] - origin[2]) / direction[2];
  return min(min(min(tx_near, tx_far), min(ty_near, ty_far)), min(tz_near, tz_far));
}