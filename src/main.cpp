#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>

#include "vector3d.h"
#include "parser.h"
#include "raytracer.h"

int main(int argc, char **argv) {
  std::unique_ptr<Scene> scene = nullptr;
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

  std::ifstream infile(argv[argc - 1]);
  if (infile) {
    scene = readDataFromStream(infile);
  } else {
    std::cerr << "Couldn't open file " << argv[argc - 1] << std::endl;
    return 1;
  }

  PNG *renderedScene = scene->render(numThreads);
  renderedScene->saveToFile(scene->filename());

  delete renderedScene;
  return 0;
}
