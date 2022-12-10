#include <iostream>
#include <fstream>

#include "lodepng.h"
#include "vector3d.h"
#include "parser.h"

using namespace std;

static Vector3D eye     (0, 0, 0); // POINT; NOT NORMALIZED
static Vector3D forward(0, 0, -1); // NOT NORMALIZED; LONGER VECTOR FOR NARROWER FIELD OF VIEW
static Vector3D right   (1, 0, 0); // NORMALIZED
static Vector3D up      (0, 1, 0); // NORMALIZED

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
    }
  } else {
    scene = readDataFromStream(cin);
  }
  cout << scene->getNumObjects() << endl;
  
  delete scene;
  return 0;
}
