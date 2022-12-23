#include <iostream>
#include <fstream>
#include <unistd.h>
#include <memory>

#include "vector3d.h"
#include "parser.h"
#include "raytracer.h"

using namespace std;

int main(int argc, char **argv) {
  unique_ptr<Scene> scene = nullptr;
  if (argc > 3) {
    cerr << "usage: " << argv[0] << " [-t numThreads] filepath" << endl;
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
        cerr << "usage: " << argv[0] << " [-t numThreads] filepath" << endl;
        return -1;
    }
  }

  ifstream infile(argv[argc - 1]);
  if (infile) {
    scene = readDataFromStream(infile);
  } else {
    cerr << "Couldn't open file " << argv[argc - 1] << endl;
    return 1;
  }

  PNG *renderedScene = scene->render(numThreads);
  renderedScene->saveToFile(scene->filename());

  delete renderedScene;
  return 0;
}
