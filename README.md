# Description
This project uses Make and clang++ with C++17.

I originally started this project as a fun exercise for myself following some guidelines from a 4 credit hour assignment in a class I took for 3 credit hours, UIUC CS 418: Interactive Computer Graphics.

The format of the scene files can be found [here](https://cs418.cs.illinois.edu/website/hw-raytracer.html) on the course site.

I plan to change the format into a more versatile one such as [PBRT's](https://www.pbrt.org/fileformat-v3).

If you would like to play around with this project, use the following commands
```
git clone [this repository]
cd [this repository]
make
./raytracer [-t numThreads] filepath
```

Any feedback or issues found are very much welcome, as well as additional contributors!

# TODOs
* Fix Global Illumination - DONE!!
    * Now samples correctly but still looks too dark - FIXED. Was clipping the color before averaging instead of the reverse

* Check if random number generator is thread safe; if not... uh oh - Unnecessary due to refactoring Object class - DONE!!

* Find a better file format or parse other formats into this format

* Implement other materials (plastic, matte surfaces, metals, ...)

    * Need interface for glossy reflections (Phong?)

* Add more benchmarking scenes (Gruesome... Unless a generous soul wants to do this by hand, I'll probably find a better file format and parse those instead)
    * Need scenes with lots of primitives, dense, overlapping, or evenly distributed throughout the scene
        * Parsed some .obj files from Stanford's 3D Repo [here](https://github.com/alecjacobson/common-3d-test-models); Format of the files was funky so I used MeshLab to convert the objs to plys and back to objs rather than figure out the parsing, I think?

* Reduce memory footprint
    * Flatten BVH. Should improve memory and performance since memory allocation is unneeded and better cache coherence - DONE!!
    
    * Also consider referencing points in the scene rather than make copies for triangles. Should help since there are ~2x as many triangles than vertices - DONE!!

    * Refactor Object class to not need independent distributions - DONE!!

    * Refactor Material class to have a supplied rng state rather than keep track of its own - DONE!!

* Might need to replace int with size_t in BVH nodes if there are lots of objects

* Parallelize SAH BVH construction - Done!!
    * Need even faster, so update BVH with binning - DONE!! Then parallelize that if possible

* Improve speed - Got nearly 33% speed up when refactoring Vector3D class
    * Align BVH nodes to cache - DONE!!

    * Refactor Vector3D to use x,y,z rather than array of floats to reduce function calls to [] operator - DONE!!

    * Refactor Object class to have public members rather than getter methods to reduce function calls - DONE!!

    * Refactor BVH intersect methods to use array-based stack to reduce std::stack overhead - DONE!! Nearly 25% speed up


* Document everything

* Refactor inputs to raytrace and illuminate to avoid long argument lists

* Create a testing suite (Should've started with this but oh well... Who actually *tests* their code?? Right? *Right??* Just Kidding!)

# Findings - M1 Macbook Pro

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

    Takes 2 minutes 14 seconds on tenthousand.txt; BVH creation takes 12.25 seconds.

    Takes 25.09 seconds on redchair.txt; BVH creation takes 0.50 seconds. NO Global Illumination.

    Takes 6 minutes 28 seconds on redchair.txt; BVH creation takes 0.49 seconds. WITH Global Illumination.

    Takes 2 minutes 23 seconds on spiral.txt; BVH creation takes 2.32 seconds.

# Current Multi-threading Render Results Using Fastest Single Threaded Method

    1 thread : Takes 1 minutes 11 seconds on tenthousand.txt.

    2 threads: Takes 37.10 seconds on tenthousand.txt.

    4 threads: Takes 19.19 seconds on tenthousand.txt.

    6 threads: Takes 15.34 seconds on tenthousand.txt.

    8 threads: Takes 11.01 seconds on tenthousand.txt.

    16 threads: Takes 12.99 seconds on tenthousand.txt.

    1 thread : Takes 3 minutes 8 seconds on redchair.txt.
    
    2 threads: Takes 1 minutes 37 seconds on redchair.txt.

    4 threads: Takes 50.36 seconds on redchair.txt.

    6 threads: Takes 39.99 seconds on redchair.txt.

    8 threads: Takes 36.52 seconds on redchair.txt.

    16 threads: Takes 34.70 seconds on redchair.txt.

    1 thread : Takes 55.55 seconds on spiral.txt.

    2 threads: Takes 28.48 seconds on spiral.txt.

    4 threads: Takes 15.07 seconds on spiral.txt.

    6 threads: Takes 11.90 seconds on spiral.txt.

    8 threads: Takes 10.68 seconds on spiral.txt.

    16 threads: Takes 10.32 seconds on spiral.txt.

# Current Multi-threading Render Results Using Fastest Single Threaded Method and Singled-threaded Binned SAH BVH

    8 threads: Takes 17.95 seconds on tenthousand.txt; BVH creation takes 0.00 seconds.

    8 threads: Takes 57.52 seconds on redchair.txt; BVH creation takes 0.01 seconds.

    8 threads: Takes 14.14 seconds on spiral.txt; BVH creation takes 0.00 seconds.

# Current Multi-threading BVH Construction Results Using Parallelized SAH loops

    4 threads: Takes 3.69 seconds on tenthousand.txt.

    6 threads: Takes 3.44 seconds on tenthousand.txt.

    8 threads: Takes 2.99 seconds on tenthousand.txt.

    16 threads: Takes 2.97 seconds on tenthousand.txt.

    4 threads: Takes 0.68 seconds on spiral.txt.

    6 threads: Takes 0.65 seconds on spiral.txt.

    8 threads: Takes 0.60 seconds on spiral.txt.

    16 threads: Takes 0.59 seconds on spiral.txt.

# Current BVH Construction Results Using Parallelized SAH loops with 8 Threads; Construction has O(N<sup>2</sup>logN)

    Takes 0.06 seconds on suzanne.obj; 968 objects.

    Takes 0.16 seconds on teapot.obj; 2256 objects.

    Takes 0.91 seconds on teapot.obj; 5804 objects.

    Takes 3 minutes 19 seconds on bunny.obj; 69451 objects.

    Takes 7 minutes 57 seconds on lucy.obj; 99970 objects.

    Did not test on dragon.obj (Estimated around 11 hours); 871414 objects.

# Current BVH Construction Results Using Binned SAH with 1 Thread and 10 Buckets and Flattening; Construction has O(NlogN)

    Takes < 0.00 seconds on suzanne.obj; 968 objects; 25.4 MB memory used.

    Takes 0.01 seconds on teapot.obj; 2256 objects; 25.8 MB memory used.

    Takes 0.01 seconds on cow.obj; 5804 objects; 26.8 MB memory used.

    Takes 0.11 seconds on bunny.obj; 69451 objects; 43.4 MB memory used.

    Takes 0.16 seconds on lucy.obj; 99970 objects; 52.5 MB memory used.

    Takes 1.66 seconds on dragon.obj; 871414 objects; 230.9 MB memory used.

    Takes 14.94 seconds on xyzrgb_dragon.obj; 7219045 objects; 1.77 GB memory used.

# Bottlenecks

findingClosestObject and findingAnyObject calls to the BVH. Given log(N) find time, each ray incurs 2log(N) cost, float a single call to the BVH. Need to improve intersection algorithm/data structure, or reduce calls.

SAH BVH building takes a very long time in scenes with many, many objects. This can be seen with tenthousand.txt where the fastest rendering method takes around 12 seconds to build the BVH. This becomes a very apparent bottleneck with multi-threading. The main thread builds the BVH in around 12 seconds, but rendering on many threads can take only around 30 seconds.

# Multi-threading strategies

## Use a queue of tasks (pixels) for threads to process

### Idea: Good for scenes with dense objects

Scenes typically will have locations of dense objects scattered around, but not evenly dispersed throughout the scene.

Queue allows each thread to process dense locations uniformly.

## Divide the scene into evenly sized chunks equal to the number of threads.

### Idea: Good for evenly dispersed scenes.

Scenes with evenly dispersed objects will take roughly the same time to finish rendering each pixel, and therefore, each chunk.

This avoids incurring synchronization costs of a thread-safe queue.

## Spawn new threads during BVH construction at each partition call

### Idea: Naive Surface Area Heuristic has N^2log(N) complexity. New threads can theoretically bring this down to Nlog(N)

In reality, this doesn't work as well, only achieving a 1 second improvement over a single threaded construction. This might be due to the initial partition call needing to view the entire scenes objects, at which point subsquent threads only see half, leading to not so significant improvements. Additionally, not all threads will be running at the beginning, leading to drastically lower improvements.

## Parallelize loop over find best SAH during BVH construction

### Idea: Divide the object search space into roughly even blocks for each thread to work on before consolidating

In practice, this method works beautifully, since each thread has its own search space and can maximize their performance on it. This allows all threads to work simultaneously at any point in the construction, and even more so towards the beginning when the search space is still massive.

# Difficulties

Multi-threading with member functions of classes. Spent a very long time figuring out how to pass arguments correctly (Need to pass in 'this' as first argument since its needed to access member variables). Also can't have variable references (&) due to move semantics.

# Current Known Bugs

Can't use more than 6 threads for some reason on redchair - FIXED: Due to a race condition in threadTaskFisheye; Was modifiying Scene's forward vector, which is shared. Solution: Make a copy of forward.

Global Illumination is not showing correct colors. Maybe improper sampling?
