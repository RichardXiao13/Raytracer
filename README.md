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

# Example Scene
![plot](./example_scenes/scene_images/example1.png)
4 minute 17.61 second 1080 by 1080 render on 1 environment light, 1 point light, 1,048,897 objects + 3 planes, and 256 rays per pixel. Credits to <a href="https://www.freepik.com/free-photo/close-up-black-marble-textured-background_3472377.htm#query=marble%20texture&position=0&from_view=keyword">marble by rawpixel.com</a> and <a href="https://www.freepik.com/free-photo/wooden_1175802.htm#query=wood%20floor%20texture&position=8&from_view=keyword">wooden by evening_tao</a> on Freepik for the textures.
