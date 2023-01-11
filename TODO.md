# TODOs - Version 2: Realistic Rendering
* Find a better file format or parse other formats into this format
    * Configured a custom format dubbed SDML (Scene Description Markup Language); Still needs a lot of work but basic interface is implemented

* Implement other materials (plastic, matte surfaces, metals, ...) and update Material interface - Copper, Gold, Plastic added

* Add more benchmarking scenes (Gruesome... Unless a generous soul wants to do this by hand, I'll probably find a better file format and parse those instead)
    * Need scenes with lots of primitives, dense, overlapping, or evenly distributed throughout the scene
        * Parsed some .obj files from Stanford's 3D Repo [here](https://github.com/alecjacobson/common-3d-test-models); Format of the files was funky so I used MeshLab to convert the objs to plys and back to objs rather than figure out the parsing

* Document everything

* Create a testing suite
