#include "macros.h"
#include "raytracer.h"
#include "BVH.h"
#include "SafeProgressBar.h"
#include "Profiler.h"

#define MIN_THREAD_WORK 64
#define N_BUCKETS 16
#define STACK_SIZE 64
#define MIN_LEAF_SIZE 4

BVH::~BVH() {
  free(nodes);
}

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
    threads[i].join();
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
    PartitionInfo info = threads[i].get();
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
    Vector3D centroid = objects[i]->centroid;
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
    extent.expand(objects[i]->centroid);
    extent.shrink(objects[i]->centroid);
  }

  const Vector3D &maxPoint = extent.maxPoint;
  const Vector3D &minPoint = extent.minPoint;

  for (int axis = 0; axis < 3; ++axis) {
    // hold extent of each bucket
    Box buckets[N_BUCKETS];
    // hold number of objects in each bucket
    int bucketCount[N_BUCKETS] = {0};
    float scale = N_BUCKETS / (maxPoint[axis] - minPoint[axis]);
    for(int i = node->start; i < end; ++i) {
      std::unique_ptr<Object> &obj = objects[i];
      // Use min to fix case where centroid is the max extent
      int bucketIdx = std::min(N_BUCKETS - 1, static_cast<int>((obj->centroid[axis] - minPoint[axis]) * scale));
      buckets[bucketIdx].shrink(obj->aabbMin);
      buckets[bucketIdx].expand(obj->aabbMax);
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
      
      leftBox.expand(buckets[i].maxPoint);
      leftBox.shrink(buckets[i].minPoint);
      leftBoxArea[i] = leftBox.surfaceArea();

      rightBox.expand(buckets[N_BUCKETS - i - 1].maxPoint);
      rightBox.shrink(buckets[N_BUCKETS - i - 1].minPoint);
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

void Box::shrink(const Vector3D& point) {
  minPoint.x = std::min(minPoint.x, point.x);
  minPoint.y = std::min(minPoint.y, point.y);
  minPoint.z = std::min(minPoint.z, point.z);
}

void Box::expand(const Vector3D& point) {
  maxPoint.x = std::max(maxPoint.x, point.x);
  maxPoint.y = std::max(maxPoint.y, point.y);
  maxPoint.z = std::max(maxPoint.z, point.z);
}

float Box::surfaceArea() {
  Vector3D extent = maxPoint - minPoint;
  // Cost uses surface area of a box, but don't need to multiply by 2 since everything is multiplied by 2 in the heuristic
  // Can remove multiply if desired
  return 2 * (extent.x * extent.y + extent.y * extent.z + extent.x * extent.z);
}

BVH::BVH(std::vector<std::unique_ptr<Object>> &objects, int maxThreads)
  : objects(objects), maxThreads(maxThreads), progress(70, objects.size(), std::max(1024.0, objects.size() * 0.01)) {
  Profiler p(Funcs::BVHConstruction);

  Node *root = new Node();
  root->start = 0;
  root->numObjects = objects.size();
  updateNodeBounds(root);
  int numNodes = partition(root);
  int idx = 0;
  // Allocate array of 32 byte blocks and align array to 32 byte address for cache performance
  nodes = (FlattenedNode *) aligned_alloc(32, 32 * numNodes);
  flatten(root, idx);
  std::cout << "BVH created with " << numNodes << " nodes on " << objects.size() << " objects." << std::endl;
  this->maxThreads += 0;
}

void BVH::updateNodeBounds(Node *node) {
  node->aabbMin = Vector3D(INF_D, INF_D, INF_D);
  node->aabbMax = Vector3D(-INF_D, -INF_D, -INF_D);
  int end = node->start + node->numObjects;
  for (int i = node->start; i < end; ++i) {
    Vector3D aabbObjMin = objects[i]->aabbMin;
    Vector3D aabbObjMax = objects[i]->aabbMax;

    node->aabbMin.x = std::min(node->aabbMin.x, aabbObjMin.x);
    node->aabbMin.y = std::min(node->aabbMin.y, aabbObjMin.y);
    node->aabbMin.z = std::min(node->aabbMin.z, aabbObjMin.z);

    node->aabbMax.x = std::max(node->aabbMax.x, aabbObjMax.x);
    node->aabbMax.y = std::max(node->aabbMax.y, aabbObjMax.y);
    node->aabbMax.z = std::max(node->aabbMax.z, aabbObjMax.z);
  }
}

float BVH::calculateSAH(Node *node, int axis, float position) {
  int leftBoxCount = 0;
  int rightBoxCount = 0;

  Box leftBox;
  Box rightBox;

  int end = node->start + node->numObjects;
  for(int i = node->start; i < end; ++i) {
    std::unique_ptr<Object> &obj = objects[i];
    if (obj->centroid[axis] < position) {
      leftBox.shrink(obj->aabbMin);
      leftBox.expand(obj->aabbMax);
      leftBoxCount++;
    }
    else {
      rightBox.shrink(obj->aabbMin);
      rightBox.expand(obj->aabbMax);
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
  if (node->numObjects <= MIN_LEAF_SIZE) {
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
    return em->centroid[axis] < splitPosition;
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
  int to_visit[STACK_SIZE];
  float distances[STACK_SIZE];
  Vector3D invDirection = 1.0f / direction;
  to_visit[0] = 0;
  distances[0] = intersectAABB(origin, invDirection, nodes[0].aabbMin, nodes[0].aabbMax);
  int stackIdx = 0;

  float minDistance = INF_D;
  IntersectionInfo closestInfo{ INF_D, {}, {}, nullptr };

  while (stackIdx >= 0) {
    float dist = distances[stackIdx];
    int nodeIdx = to_visit[stackIdx];
    FlattenedNode &subtree = nodes[nodeIdx];
    if (dist < minDistance) {
      if (subtree.isLeaf()) {
        int end = subtree.start + subtree.numObjects;
        for (int i = subtree.start; i < end; ++i) {
          IntersectionInfo info = objects[i]->intersect(origin, direction);
          if (info.t < minDistance) {
            minDistance = info.t;
            closestInfo = info;
          }
        }
      } else {
        int leftIdx = nodeIdx + 1;
        int rightIdx = subtree.right;
        FlattenedNode &left = nodes[leftIdx];
        FlattenedNode &right = nodes[rightIdx];
        float leftDistance = intersectAABB(origin, invDirection, left.aabbMin, left.aabbMax);
        float rightDistance = intersectAABB(origin, invDirection, right.aabbMin, right.aabbMax);
        if (rightDistance < leftDistance) {
          std::swap(leftIdx, rightIdx);
          std::swap(leftDistance, rightDistance);
        }
        if (leftDistance != INF_D) {
          if (rightDistance != INF_D) {
            to_visit[stackIdx] = rightIdx;
            distances[stackIdx++] = rightDistance;
          }
          to_visit[stackIdx] = leftIdx;
          distances[stackIdx++] = leftDistance;
        }
      }
    }
    --stackIdx;
  }

  return closestInfo;
}

bool BVH::findAnyObject(const Vector3D& origin, const Vector3D& direction) {
  #ifdef PROFILE_INTERSECT
  Profiler p(Funcs::BVHIntersectAny);
  #endif
  // Use array which is lighter and faster than vector/stack
  // to_visit stores next node index to visit
  int to_visit[STACK_SIZE];
  Vector3D invDirection = 1.0f / direction;
  to_visit[0] = 0;
  int stackIdx = 0;

  while (stackIdx >= 0) {
    int nodeIdx = to_visit[stackIdx];
    FlattenedNode &subtree = nodes[nodeIdx];
    if (subtree.isLeaf()) {
      int end = subtree.start + subtree.numObjects;
      for (int i = subtree.start; i < end; ++i) {
        IntersectionInfo info = objects[i]->intersect(origin, direction);
        if (info.obj != nullptr) {
          return true;
        }
      }
    } else {
      int leftIdx = nodeIdx + 1;
      int rightIdx = subtree.right;
      FlattenedNode &left = nodes[leftIdx];
      FlattenedNode &right = nodes[rightIdx];
      float leftDistance = intersectAABB(origin, invDirection, left.aabbMin, left.aabbMax);
      float rightDistance = intersectAABB(origin, invDirection, right.aabbMin, right.aabbMax);
      if (rightDistance < leftDistance) {
        std::swap(leftIdx, rightIdx);
        std::swap(leftDistance, rightDistance);
      }
      if (leftDistance != INF_D) {
        if (rightDistance != INF_D) {
          to_visit[stackIdx++] = rightIdx;
        }
        to_visit[stackIdx++] = leftIdx;
      }
    }
    --stackIdx;
  }

  return false;
}

float BVH::intersectAABB(const Vector3D& origin, const Vector3D& invDirection, const Vector3D& aabbMin, const Vector3D& aabbMax) {
  // Profiler p(Funcs::BVHIntersectAABB);

  float tx_near = (aabbMin.x - origin.x) * invDirection.x;
  float tx_far = (aabbMax.x - origin.x) * invDirection.x;
  float tmin = std::min(tx_near, tx_far);
  float tmax = std::max(tx_near, tx_far);
  float ty_near = (aabbMin.y - origin.y) * invDirection.y;
  float ty_far = (aabbMax.y - origin.y) * invDirection.y;
  tmin = std::max(tmin, std::min(ty_near, ty_far));
  tmax = std::min(tmax, std::max(ty_near, ty_far));
  float tz_near = (aabbMin.z - origin.z) * invDirection.z;
  float tz_far = (aabbMax.z - origin.z) * invDirection.z;
  tmin = std::max(tmin, std::min(tz_near, tz_far));
  tmax = std::min(tmax, std::max(tz_near, tz_far));
  if (tmax >= tmin && tmax > 0) {
    return tmin;
  }
  return INF_D;
}

void BVH::flatten(Node *node, int &idx) {
  // i is index of current node in the array
  int i = idx;
  ++idx;
  FlattenedNode &flatNode = nodes[i];
  flatNode.aabbMin = node->aabbMin;
  flatNode.aabbMax = node->aabbMax;
  flatNode.numObjects = node->numObjects;
  if (node->isLeaf()) {
    flatNode.start = node->start;
    delete node;
    return;
  }
  flatten(node->left, idx);
  flatNode.right = idx;
  flatten(node->right, idx);
  delete node;
}
