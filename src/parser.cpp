#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <iostream>

#include "parser.h"

using namespace std;

void Scene::addObject(Object *obj) {
  objects.push_back(obj);
}

size_t Scene::getNumObjects() {
  return objects.size();
}

Scene::~Scene() {
  for (auto it = objects.begin(); it != objects.end(); ++it) {
    delete *it;
  }
}

template <typename Out>
void split(const string &s, char delim, Out result) {
    istringstream iss(s);
    string item;
    while (getline(iss, item, delim)) {
        *result++ = item;
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

      scene->addObject(new Sphere(x, y, z, r));
    }
  }

  return scene;
}