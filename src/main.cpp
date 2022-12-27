#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>

#include "vector3d.h"
#include "parser.h"
#include "raytracer.h"
#include "Profiler.h"

int main(int argc, char **argv) {
  if (argc > 3) {
    std::cerr << "usage: " << argv[0] << " [-t numThreads] filepath" << std::endl;
    return 1;
  }

  int opt;
  int numThreads = 4;
  while ((opt = getopt(argc, argv, "t:")) != -1) {
    switch (opt) {
      case 't':
        numThreads = atoi(optarg);
        break;
      default:
        std::cerr << "usage: " << argv[0] << " [-t numThreads] filepath" << std::endl;
        return -1;
    }
  }

  std::unique_ptr<Scene> scene = readFromFile(argv[argc - 1]);
  if (!scene) {
    return 1;
  }

  PNG *renderedScene = scene->render(numThreads);
  renderedScene->saveToFile(scene->filename());
  printStats();
  delete renderedScene;
  return 0;
}
