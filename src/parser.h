#pragma once

#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>

using namespace std;

class Object {};

class Sphere : public Object {
public:
  Sphere(double x1, double y1, double z1, double r1) : x(x1), y(y1), z(z1), r(r1) {};

  double x;
  double y;
  double z;
  double r;
};

class Scene {
public:
  Scene(int w, int h, const string& file) : width(w), height(h), filename(file) {};
  ~Scene();
  void addObject(Object *obj);
  size_t getNumObjects();

  int width;
  int height;
  string filename;

private:
  vector<Object*> objects;
};

template <typename Out>
void split(const string &s, char delim, Out result);

vector<string> split(const string &s, char delim);

Scene *readDataFromStream(std::istream& in);