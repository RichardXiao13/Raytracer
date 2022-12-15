#include <iostream>
#include <fstream>

#include "vector3d.h"
#include "parser.h"
#include "raytracer.h"

using namespace std;

int main(int argc, char **argv) {
  Scene *scene = nullptr;
  if (argc > 2) {
    cerr << "usage: ./raytracer [filename]" << endl;
    return 1;
  } else if (argc == 2) {
    ifstream infile(argv[1]);
    if (infile) {
      scene = readDataFromStream(infile);
    } else {
      cerr << "Couldn't open file " << argv[1] << endl;
      return 1;
    }
  } else {
    scene = readDataFromStream(cin);
  }

  PNG *renderedScene = scene->render();
  renderedScene->saveToFile(scene->filename());

  delete renderedScene;
  delete scene;
  return 0;
}
