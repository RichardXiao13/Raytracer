# Findings

Naive implementation of finding closest object

    Takes around 20 hours on tenthousand.txt

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using Midpoint split and Priority Queue traversal: O(NlogN) for traversing - NO Compiler Optimizations

    Takes 72 minutes 19 seconds on tenthousand.txt

    Takes 23 minutes 16 seconds on redchair.txt. NO Global Illumination.

    Takes 77 minutes 29 seconds on spiral.txt

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic and Priority Queue traversal: O(NlogN) for traversing - NO Compiler Optimizations

    Takes 64 minutes 46 seconds on tenthousand.txt; BVH creation takes 39.76 seconds.

    Takes 7 minutes 37 seconds on redchair.txt; BVH creation takes 1.59 seconds. NO Global Illumination.

    Takes 70 minutes 13 seconds on spiral.txt; BVH creation takes 9.63 seconds.

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic and Stack traversal: O(logN) for traversing - NO Compiler Optimizations

    Takes 35 minutes 41 seconds on tenthousand.txt; BVH creation takes 40.04 seconds.

    Takes 4 minutes 43 seconds on redchair.txt; BVH creation takes 1.62 seconds. NO Global Illumination.

    Takes 11 minutes 48 seconds on redchair.txt; BVH creation takes 1.60 seconds. WITH Global Illumination.

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic, Stack traversal, and early shadow breaking: O(logN) for traversing - NO Compiler Optimizations

    Takes 16 minutes 19 seconds on tenthousand.txt; BVH creation takes 40.02 seconds.

    Takes 2 minutes 41 seconds on redchair.txt; BVH creation takes 1.62 seconds. NO Global Illumination.
    
    Takes 42 minutes 40 seconds on redchair.txt; BVH creation takes 1.61 seconds. WITH Global Illumination.

    Takes 14 minutes 17 seconds on spiral.txt; BVH creation takes 9.49 seconds.

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic, Stack traversal, and early shadow breaking: O(logN) for traversing - WITH Compiler Optimizations -O3

    Takes 2 minutes 33 seconds on tenthousand.txt; BVH creation takes 12.25 seconds.

    Takes 25.09 seconds on redchair.txt; BVH creation takes 0.50 seconds. NO Global Illumination.

    Takes 5 minutes 33 seconds on redchair.txt; BVH creation takes 0.49 seconds. WITH Global Illumination.

    Takes 2 minutes 23 seconds on spiral.txt; BVH creation takes 2.32 seconds.

# Multi-threading Results Using Fastest Single Threaded Method

4 threads: Takes 37.15 seconds on spiral.txt.

6 threads: Takes 31.62 seconds on spiral.txt.

8 threads: Takes 29.98 seconds on spiral.txt.

16 threads: Takes 29.06 seconds on spiral.txt.

# Bottlenecks

findingClosestObject and findingAnyObject calls to the BVH. Given log(N) find time, each ray incurs 2log(N) cost, double a single call to the BVH. Need to improve intersection algorithm/data structure, or reduce calls.

# Multi-threading strategies

## Use a queue of tasks (pixels) for threads to process

### Idea: Good for scenes with dense objects

Scenes typically will have locations of dense objects scattered around, but not evenly dispersed throughout the scene.

Queue allows each thread to process dense locations uniformly.

## Divide the scene into evenly sized chunks equal to the number of threads.

### Idea: Good for evenly dispersed scenes.

Scenes with evenly dispersed objects will take roughly the same time to finish rendering each pixel, and therefore, each chunk.

This avoids incurring synchronization costs of a thread-safe queue.

# Difficulties

Multi-threading with member functions of classes. Spent a very long time figuring out how to pass arguments correctly (Need to pass in 'this' as first argument since its needed to access member variables). Also can't have variable references (&) due to move semantics.

# Current Known Bugs

Can't use more than 6 threads for some reason on redchair - FIXED: Due to a race condition in threadTaskFisheye; Was modifiying Scene's forward vector, which is shared. Solution: Make a copy of forward.

Global Illumination is not showing correct colors. Maybe improper sampling?
