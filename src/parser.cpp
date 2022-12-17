#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "parser.h"
#include "raytracer.h"
#include "Objects.h"

using namespace std;

template <typename Out>
void split(const string &s, char delim, Out result) {
  istringstream iss(s);
  string item;
  while (getline(iss, item, delim)) {
    if (!item.empty()) {
      *result++ = item;
    }
  }
}

vector<string> split(const string &s, char delim) {
  vector<string> elems;
  split(s, delim, back_inserter(elems));
  return elems;
}

Scene *readDataFromStream(istream& in) {
  string line;
  getline(in, line);
  vector<string> lineInfo = split(line, ' ');

  if (lineInfo.size() != 4) {
    cerr << "Supplied PNG info doesn't have the correct number of arguments. Expected 4. Got "
         << lineInfo.size()
         << '.'
         << endl;
    return nullptr;
  } else if (lineInfo.at(0) != "png") {
    cerr << "Expected PNG image type. Got " << lineInfo.at(0) << '.' << endl;
    return nullptr;
  }

  int width = stoi(lineInfo.at(1));
  int height = stoi(lineInfo.at(2));
  string filename = lineInfo.at(3);
  Scene *scene = new Scene(width, height, filename);

  RGBAColor currentColor(1, 1, 1, 1);
  Vector3D currentShine(0, 0, 0);
  Vector3D currentTransparency(0, 0, 0);
  double currentIOR = 1.458;
  double currentRoughness = 0.0;

  for (; getline(in, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    string keyword = lineInfo.at(0);

    if (keyword == "sphere") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      double r = stod(lineInfo.at(4));
      Sphere *newObject = new Sphere(x, y, z, r);
      newObject->setColor(currentColor);
      newObject->setShine(currentShine);
      newObject->setTransparency(currentTransparency);
      newObject->setIndexOfRefraction(currentIOR);
      newObject->setRoughness(currentRoughness);
      scene->addObject(newObject);
    } else if (keyword == "sun") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      Light *newLight = new Light(x, y, z, currentColor);
      scene->addLight(newLight);
    } else if (keyword == "color") {
      double r = stod(lineInfo.at(1));
      double g = stod(lineInfo.at(2));
      double b = stod(lineInfo.at(3));
      currentColor = RGBAColor(r, g, b, 1);
    } else if (keyword == "plane") {
      double A = stod(lineInfo.at(1));
      double B = stod(lineInfo.at(2));
      double C = stod(lineInfo.at(3));
      double D = stod(lineInfo.at(4));
      Plane *newObject = new Plane(A, B, C, D);
      newObject->setColor(currentColor);
      newObject->setShine(currentShine);
      newObject->setTransparency(currentTransparency);
      newObject->setIndexOfRefraction(currentIOR);
      newObject->setRoughness(currentRoughness);
      scene->addPlane(newObject);
    } else if (keyword == "bulb") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      Bulb *newBulb = new Bulb(x, y, z, currentColor);
      scene->addBulb(newBulb);
    } else if (keyword == "xyz") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      scene->addPoint(x, y, z);
    } else if (keyword == "trif") {
      int i = stoi(lineInfo.at(1)) - 1;
      int j = stoi(lineInfo.at(2)) - 1;
      int k = stoi(lineInfo.at(3)) - 1;
      Triangle *newObject = new Triangle(scene->getPoint(i), scene->getPoint(j), scene->getPoint(k));
      newObject->setColor(currentColor);
      newObject->setShine(currentShine);
      newObject->setTransparency(currentTransparency);
      newObject->setIndexOfRefraction(currentIOR);
      newObject->setRoughness(currentRoughness);
      scene->addObject(newObject);
    } else if (keyword == "expose") {
      double exposure = stod(lineInfo.at(1));
      scene->setExposure(exposure);
    } else if (keyword == "shininess") {
      double Sr = stod(lineInfo.at(1));
      double Sg = Sr;
      double Sb = Sr;
      if (lineInfo.size() > 2) {
        Sg = stod(lineInfo.at(2));
        Sb = stod(lineInfo.at(3));
      }
      currentShine = Vector3D(Sr, Sg, Sb);
    } else if (keyword == "bounces") {
      double d = stoi(lineInfo.at(1));
      scene->setMaxBounces(d);
    } else if (keyword == "transparency") {
      double Tr = stod(lineInfo.at(1));
      double Tg = Tr;
      double Tb = Tr;
      if (lineInfo.size() > 2) {
        Tg = stod(lineInfo.at(2));
        Tb = stod(lineInfo.at(3));
      }
      currentTransparency = Vector3D(Tr, Tg, Tb);
    } else if (keyword == "aa") {
      int n = stoi(lineInfo.at(1));
      scene->setNumRays(n);
    } else if (keyword == "roughness") {
      double roughness = stod(lineInfo.at(1));
      currentRoughness = roughness;
    } else if (keyword == "eye") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      scene->setEye(Vector3D(x, y, z));
    } else if (keyword == "forward") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      scene->setForward(Vector3D(x, y, z));
    } else if (keyword == "up") {
      double x = stod(lineInfo.at(1));
      double y = stod(lineInfo.at(2));
      double z = stod(lineInfo.at(3));
      scene->setUp(Vector3D(x, y, z));
    } else if (keyword == "fisheye") {
      scene->enableFisheye();
    } else if (keyword == "ior") {
      double ior = stod(lineInfo.at(1));
      currentIOR = ior;
    } else if (keyword == "gi") {
      int gi = stoi(lineInfo.at(1));
      scene->setGlobalIllumination(gi);
    }
  }

  return scene;
}
