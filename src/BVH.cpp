#include <vector>
#include <queue>
#include <functional>
#include <iostream>
#include <stack>
#include <memory>
#include <thread>
#include <future>
#include <algorithm>
#include <iterator>
#include <cmath>

#include "raytracer.h"
#include "BVH.h"
#include "SafeProgressBar.h"
#include "Profiler.h"
#include "macros.h"

#define MIN_THREAD_WORK 64
#define N_BUCKETS 10
#define STACK_SIZE 64

void parallelFor(int start, int end, int maxThreads, std::function<void (int)> func) {
  int workPerThread = std::max((end - start) / maxThreads, MIN_THREAD_WORK);
  std::vector<std::thread> threads;
  for (int i = start; i < end; i += workPerThread) {
    threads.emplace_back([&](int loopStart, int loopEnd) {
      for (; loopStart < loopEnd; ++loopStart) {
        func(loopStart);
      }
    }, i, std::min(i + workPerThread, end));
  }
  for (size_t i = 0; i < threads.size(); ++i) {
    threads.at(i).join();
  }
}

PartitionInfo BVH::parallelizeSAH(Node *node, int start, int end, int maxThreads) {
  int workPerThread = std::max((end - start) / maxThreads, MIN_THREAD_WORK);
  // Use futures as if they were threads. May not always be the case, but this is a nice abstraction
  std::vector<std::future<PartitionInfo>> threads;
  for (int i = start; i < end; i += workPerThread) {
    threads.emplace_back(
      std::async(&BVH::threadPartitionTask, this, node, i, std::min(i + workPerThread, end))
    );
  }
  PartitionInfo bestInfo;
  for (size_t i = 0; i < threads.size(); ++i) {
    PartitionInfo info = threads.at(i).get();
    if (info.bestCost < bestInfo.bestCost) {
      bestInfo = info;
    }
  }
  return bestInfo;
}

PartitionInfo BVH::threadPartitionTask(Node *node, int start, int end) {
  int bestAxis = -1;
  float bestPosition = 0;
  float bestCost = INF_D;

  for (int i = start; i < end; ++i) {
    Vector3D centroid = objects.at(i)->centroid();
    for (int axis = 0; axis < 3; ++axis) {
      float possiblePosition = centroid[axis];
      float cost = calculateSAH(node, axis, possiblePosition);
      if (cost < bestCost) {
        bestCost = cost;
        bestAxis = axis;
        bestPosition = possiblePosition;
      }
    }
  }
  
  return { bestAxis, bestPosition, bestCost };
}

PartitionInfo BVH::findBestBucketSplit(Node *node) {
  int bestAxis = -1;
  float bestPosition = 0;
  float bestCost = INF_D;

  // Find centroid extent of node
  Box extent;
  int end = node->start + node->numObjects;
  for(int i = node->start; i < end; ++i) {
    extent.expand(objects.at(i)->centroid());
    extent.shrink(objects.at(i)->centroid());
  }

  const Vector3D &maxPoint = extent.maxPoint();
  const Vector3D &minPoint = extent.minPoint();

  for (int axis = 0; axis < 3; ++axis) {
    // hold extent of each bucket
    Box buckets[N_BUCKETS];
    // hold number of objects in each bucket
    int bucketCount[N_BUCKETS] = {0};
    float scale = N_BUCKETS / (maxPoint[axis] - minPoint[axis]);
    for(int i = node->start; i < end; ++i) {
      std::unique_ptr<Object> &obj = objects.at(i);
      // Use min to fix case where centroid is the max extent
      int bucketIdx = std::min(N_BUCKETS - 1, static_cast<int>((obj->centroid()[axis] - minPoint[axis]) * scale));
      buckets[bucketIdx].shrink(obj->aabbMin());
      buckets[bucketIdx].expand(obj->aabbMax());
      bucketCount[bucketIdx]++;
    }

    int    leftBoxCount[N_BUCKETS - 1] = {0};
    int   rightBoxCount[N_BUCKETS - 1] = {0};
    float  leftBoxArea[N_BUCKETS - 1] = {0};
    float rightBoxArea[N_BUCKETS - 1] = {0};
    int leftCount = 0;
    int rightCount = 0;
    Box leftBox;
    Box rightBox;
    // compute box counts using prefix and suffix sums
    for (int i = 0; i < N_BUCKETS - 1; ++i) {
      leftCount += bucketCount[i];
      leftBoxCount[i] = leftCount;
      rightCount += bucketCount[N_BUCKETS - i - 1];
      rightBoxCount[N_BUCKETS - i - 2] = rightCount;
      
      leftBox.expand(buckets[i].maxPoint());
      leftBox.shrink(buckets[i].minPoint());
      leftBoxArea[i] = leftBox.surfaceArea();

      rightBox.expand(buckets[N_BUCKETS - i - 1].maxPoint());
      rightBox.shrink(buckets[N_BUCKETS - i - 1].minPoint());
      rightBoxArea[N_BUCKETS - i - 2] = rightBox.surfaceArea();
    }

    scale = (maxPoint[axis] - minPoint[axis]) / N_BUCKETS;
    for (int i = 0; i < N_BUCKETS - 1; ++i) {
      // cost is left box area * left box count + right box area * right box count
      float cost = leftBoxArea[i] * leftBoxCount[i] + rightBoxArea[i] * rightBoxCount[i];
      if (cost < bestCost) {
        bestCost = cost;
        bestAxis = axis;
        bestPosition = minPoint[axis] + (i + 1) * scale;
      }
    }
  }
  
  return { bestAxis, bestPosition, bestCost };
}

void Box::shrink(const Vector3D& minPoint) {
  minPoint_[0] = std::min(minPoint_[0], minPoint[0]);
  minPoint_[1] = std::min(minPoint_[1], minPoint[1]);
  minPoint_[2] = std::min(minPoint_[2], minPoint[2]);
}

void Box::expand(const Vector3D& maxPoint) {
  maxPoint_[0] = std::max(maxPoint_[0], maxPoint[0]);
  maxPoint_[1] = std::max(maxPoint_[1], maxPoint[1]);
  maxPoint_[2] = std::max(maxPoint_[2], maxPoint[2]);
}

float Box::surfaceArea() {
  Vector3D extent = maxPoint_ - minPoint_;
  // Cost uses surface area of a box, but don't need to multiply by 2 since everything is multiplied by 2 in the heuristic
  // Can remove multiply if desired
  return 2 * (extent[0] * extent[1] + extent[1] * extent[2] + extent[0] * extent[2]);
}

BVH::BVH(std::vector<std::unique_ptr<Object>> &objects, int maxThreads)
  : objects(objects), maxThreads(maxThreads), progress(70, objects.size(), std::max(1024.0, objects.size() * 0.01)) {
  Profiler p(Funcs::BVHConstruction);

  Node *root = new Node();
  root->start = 0;
  root->numObjects = objects.size();
  updateNodeBounds(root);

  int numNodes = partition(root);
  std::cout << "Flattening BVH with " << numNodes << " nodes" << std::endl;
  int idx = 0;
  nodes.reserve(numNodes);
  flatten(root, idx);
  this->maxThreads += 0;
}

void BVH::updateNodeBounds(Node *node) {
  node->aabbMin = Vector3D(INF_D, INF_D, INF_D);
  node->aabbMax = Vector3D(-INF_D, -INF_D, -INF_D);
  int end = node->start + node->numObjects;
  for (int i = node->start; i < end; ++i) {
    Vector3D aabbObjMin = objects.at(i)->aabbMin();
    Vector3D aabbObjMax = objects.at(i)->aabbMax();

    node->aabbMin[0] = std::min(node->aabbMin[0], aabbObjMin[0]);
    node->aabbMin[1] = std::min(node->aabbMin[1], aabbObjMin[1]);
    node->aabbMin[2] = std::min(node->aabbMin[2], aabbObjMin[2]);

    node->aabbMax[0] = std::max(node->aabbMax[0], aabbObjMax[0]);
    node->aabbMax[1] = std::max(node->aabbMax[1], aabbObjMax[1]);
    node->aabbMax[2] = std::max(node->aabbMax[2], aabbObjMax[2]);
  }
}

float BVH::calculateSAH(Node *node, int axis, float position) {
  int leftBoxCount = 0;
  int rightBoxCount = 0;

  Box leftBox;
  Box rightBox;

  int end = node->start + node->numObjects;
  for(int i = node->start; i < end; ++i) {
    std::unique_ptr<Object> &obj = objects.at(i);
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

  float cost = leftBoxCount * leftBox.surfaceArea() + rightBoxCount * rightBox.surfaceArea();
  if (cost > 0) {
    return cost;
  }
  return INF_D;
}

int BVH::partition(Node *node) {
  // Base case: allow a max of 4 objects for a node before becoming a leaf
  if (node->numObjects <= 4) {
    progress.increment(node->numObjects);
    return 1;
  }
  
  int bestAxis = -1;
  float bestPosition = 0;
  float bestCost = INF_D;
  int end = node->start + node->numObjects;

  // PartitionInfo info = parallelizeSAH(node, node->start, end, maxThreads);
  PartitionInfo info = findBestBucketSplit(node);

  Box parentBox(node->aabbMin, node->aabbMax);
  float parentCost = parentBox.surfaceArea() * node->numObjects;
  if (info.bestCost >= parentCost) {
    progress.increment(node->numObjects);
    return 1;
  }

  int axis = info.bestAxis;
  float splitPosition = info.bestPosition;
  int i = node->start;
  int j = i + node->numObjects;
  
  auto middle = std::partition(objects.begin() + i, objects.begin() + j,
  [axis, splitPosition](const std::unique_ptr<Object>& em) {
    return em->centroid()[axis] < splitPosition;
  });

  // abort split if one of the sides is empty
  int leftNumObjects = std::distance(objects.begin() + i, middle);
  if (leftNumObjects == 0 || leftNumObjects == node->numObjects) {
    progress.increment(node->numObjects);
    return 1;
  }
  // create child nodes
  node->left = new Node();
  node->right = new Node();
  node->left->start = node->start;
  node->left->numObjects = leftNumObjects;
  node->right->start = i + leftNumObjects;
  node->right->numObjects = node->numObjects - leftNumObjects;
  node->numObjects = 0;
  updateNodeBounds(node->left);
  updateNodeBounds(node->right);

  return partition(node->left) + partition(node->right) + 1;
}

IntersectionInfo BVH::findClosestObject(const Vector3D& origin, const Vector3D& direction) {
  #ifdef PROFILE_INTERSECT
  Profiler p(Funcs::BVHIntersectClosest);
  #endif
  // Use vector as the underlying container
  // Better/faster for operations on the top of the stack, which is all this function does
  std::stack<std::pair<float, int>, std::vector<std::pair<float, int>>> to_visit;
  to_visit.push({ intersectAABB(origin, direction, nodes.at(0).aabbMin, nodes.at(0).aabbMax), 0 });

  float minDistance = INF_D;
  IntersectionInfo closestInfo{ INF_D, {}, {}, nullptr };

  while (to_visit.empty() == false) {
    std::pair<float, int> pairing = to_visit.top();
    float dist = pairing.first;
    FlattenedNode &subtree = nodes.at(pairing.second);
    to_visit.pop();
    if (dist < minDistance) {
      if (subtree.isLeaf()) {
        int end = subtree.start + subtree.numObjects;
        for (int i = subtree.start; i < end; ++i) {
          IntersectionInfo info = objects.at(i)->intersect(origin, direction);
          if (info.t < minDistance) {
            minDistance = info.t;
            closestInfo = info;
          }
        }
      } else {
        int leftIdx = subtree.left;
        int rightIdx = subtree.right;
        FlattenedNode &left = nodes.at(leftIdx);
        FlattenedNode &right = nodes.at(rightIdx);
        float leftDistance = intersectAABB(origin, direction, left.aabbMin, left.aabbMax);
        float rightDistance = intersectAABB(origin, direction, right.aabbMin, right.aabbMax);
        if (rightDistance < leftDistance) {
          std::swap(leftIdx, rightIdx);
          std::swap(leftDistance, rightDistance);
        }
        if (leftDistance != INF_D) {
          if (rightDistance != INF_D) {
            to_visit.push({ rightDistance, rightIdx });
          }
          to_visit.push({ leftDistance, leftIdx });
        }
      }
    }
  }

  return closestInfo;
}

bool BVH::findAnyObject(const Vector3D& origin, const Vector3D& direction) {
  #ifdef PROFILE_INTERSECT
  Profiler p(Funcs::BVHIntersectAny);
  #endif
  // Use vector as the underlying container
  // Better/faster for operations on the top of the stack, which is all this function does
  std::stack<std::pair<float, int>, std::vector<std::pair<float, int>>> to_visit;
  to_visit.push({ intersectAABB(origin, direction, nodes.at(0).aabbMin, nodes.at(0).aabbMax), 0 });

  float minDistance = INF_D;

  while (to_visit.empty() == false) {
    std::pair<float, int> pairing = to_visit.top();
    float dist = pairing.first;
    FlattenedNode &subtree = nodes.at(pairing.second);
    to_visit.pop();
    if (dist < minDistance) {
      if (subtree.isLeaf()) {
        int end = subtree.start + subtree.numObjects;
        for (int i = subtree.start; i < end; ++i) {
          IntersectionInfo info = objects.at(i)->intersect(origin, direction);
          if (info.obj != nullptr) {
            return true;
          }
        }
      } else {
        int leftIdx = subtree.left;
        int rightIdx = subtree.right;
        FlattenedNode &left = nodes.at(leftIdx);
        FlattenedNode &right = nodes.at(rightIdx);
        float leftDistance = intersectAABB(origin, direction, left.aabbMin, left.aabbMax);
        float rightDistance = intersectAABB(origin, direction, right.aabbMin, right.aabbMax);
        if (rightDistance < leftDistance) {
          std::swap(leftIdx, rightIdx);
          std::swap(leftDistance, rightDistance);
        }
        if (leftDistance != INF_D) {
          if (rightDistance != INF_D) {
            to_visit.push({ rightDistance, rightIdx });
          }
          to_visit.push({ leftDistance, leftIdx });
        }
      }
    }
  }

  return false;
}

float BVH::intersectAABB(const Vector3D& origin, const Vector3D& direction, const Vector3D& aabbMin, const Vector3D& aabbMax) {
  // Profiler p(Funcs::BVHIntersectAABB);

  float tx_near = (aabbMin[0] - origin[0]) / direction[0];
  float tx_far = (aabbMax[0] - origin[0]) / direction[0];
  float tmin = std::min(tx_near, tx_far);
  float tmax = std::max(tx_near, tx_far);
  float ty_near = (aabbMin[1] - origin[1]) / direction[1];
  float ty_far = (aabbMax[1] - origin[1]) / direction[1];
  tmin = std::max(tmin, std::min(ty_near, ty_far));
  tmax = std::min(tmax, std::max(ty_near, ty_far));
  float tz_near = (aabbMin[2] - origin[2]) / direction[2];
  float tz_far = (aabbMax[2] - origin[2]) / direction[2];
  tmin = std::max(tmin, std::min(tz_near, tz_far));
  tmax = std::min(tmax, std::max(tz_near, tz_far));
  if (tmax >= tmin && tmax > 0) {
    return tmin;
  }
  return INF_D;
}

int BVH::size() {
  return nodes.size();
}

void BVH::flatten(Node *node, int &idx) {
  // i is index of current node in the array
  int i = idx;
  ++idx;
  nodes.push_back({node->aabbMin, node->aabbMax, idx, 0, node->start, node->numObjects});
  if (node->isLeaf()) {
    delete node;
    return;
  }
  flatten(node->left, idx);
  nodes.at(i).right = idx;
  flatten(node->right, idx);
  delete node;
}