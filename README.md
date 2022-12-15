# Findings

Naive implementation of finding closest object takes an estimated 20 hours on tenthousand.txt

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using Midpoint split and Priority Queue traversal

    Takes 72 minutes 19 seconds on tenthousand.txt

    Takes 23 minutes 16 seconds on redchair.txt. NO Global Illumination.

    Takes 77 minutes 29 seconds on spiral.txt

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic and Priority Queue traversal

    Takes 64 minutes 46 seconds on tenthousand.txt; BVH creation takes 39.76 seconds.

    Takes 7 minutes 37 seconds on redchair.txt; BVH creation takes 1.59 seconds. NO Global Illumination.

    Takes 70 minutes 13 seconds on tenthousand.txt; BVH creation takes 9.63 seconds.

Bounding Volume Hierarchy: Axis-Aligned Bounding Box using the Surface Area Heuristic and Stack traversal

    Takes 35 minutes 41 seconds on tenthousand.txt; BVH creation takes 50.04 seconds.

    Takes 4 minutes 43 seconds on redchair.txt; BVH creation takes 1.62 seconds. NO Global Illumination.