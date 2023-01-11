# Description
This project uses Make and clang++ with C++17.

I originally started this project as a fun exercise for myself following some guidelines from a 4 credit hour assignment in a class I took for 3 credit hours, UIUC CS 418: Interactive Computer Graphics.

The format of the scene files can be found [here](https://cs418.cs.illinois.edu/website/hw-raytracer.html) on the course site.

Note that Version 2 is transitioning away from the txt-based scene files while integrating a custom format that can be found in FileFormat.md. While some functionality still works with the original scene files, most has shifted to a different model of rendering.

Benchmarks from Version 1 can be found in benchmarks. This includes rendering times with Bounding Volume Hierarchy acceleration and multi-threading.

If you would like to play around with this project, use the following commands
```
git clone [this repository]
cd [this repository]
make
./raytracer [-t numThreads] filepath
```

Any feedback or issues found are very much welcome, as well as additional contributors!

# TODOs - Version 2: Realistic Rendering
* Add environment lighting - Need to figure out shadowing

* Add textures - Fix textures for planes not flat with xy plane

* Find a better file format or parse other formats into this format
    * Configured a custom format dubbed SDML (Scene Description Markup Language); Still needs a lot of work but basic interface is implemented for all except textures

* Implement other materials (plastic, matte surfaces, metals, ...) and update Material interface - Copper, Gold, Plastic added

* Add more benchmarking scenes (Gruesome... Unless a generous soul wants to do this by hand, I'll probably find a better file format and parse those instead)
    * Need scenes with lots of primitives, dense, overlapping, or evenly distributed throughout the scene
        * Parsed some .obj files from Stanford's 3D Repo [here](https://github.com/alecjacobson/common-3d-test-models); Format of the files was funky so I used MeshLab to convert the objs to plys and back to objs rather than figure out the parsing, I think?

* Document everything

* Create a testing suite
