#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>

#include "parser.h"
#include "raytracer.h"

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
  double currentShine = 0;

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
      scene->addObject(newObject);
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
      scene->addObject(newObject);
    } else if (keyword == "expose") {
      double exposure = stod(lineInfo.at(1));
      scene->setExposure(exposure);
    } else if (keyword == "shininess") {
      double s = stod(lineInfo.at(1));
      currentShine = s;
    }
  }

  return scene;
}
