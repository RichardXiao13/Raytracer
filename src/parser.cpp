#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <iterator>
#include <iostream>
#include <algorithm>
#include <memory>
#include <random>

#include "parser.h"
#include "raytracer.h"
#include "Objects.h"
#include "materials/Material.h"
#include "materials/Glass.h"

// From StackOverflow https://stackoverflow.com/questions/874134/find-out-if-string-ends-with-another-string-in-c
inline bool ends_with(std::string const & value, std::string const & ending) {
  if (ending.size() > value.size()) return false;
  return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

template <typename Out>
void split(const std::string &s, char delim, Out result) {
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, delim)) {
    if (!item.empty()) {
      *result++ = item;
    }
  }
}

std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> elems;
  split(s, delim, std::back_inserter(elems));
  return elems;
}

std::unique_ptr<Scene> readOBJ(std::istream& in) {
  std::string line;
  std::getline(in, line);
  std::vector<std::string> lineInfo = split(line, ' ');

  // Will set scene filename later
  std::unique_ptr<Scene> scene = std::make_unique<Scene>(512, 512, "");
  std::unique_ptr<Material> currentMaterial = std::make_unique<Material>(Vector3D(0,0,0), Vector3D(0,0,0), 1.458, 0.0);
  RGBAColor currentColor(1, 1, 1, 1);

  for (; std::getline(in, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "v") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      scene->addPoint(x, y, z);
    } else if (keyword == "f") {
      int i = std::stoi(lineInfo.at(1)) - 1;
      int j = std::stoi(lineInfo.at(2)) - 1;
      int k = std::stoi(lineInfo.at(3)) - 1;
      std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(scene->getPoint(i), scene->getPoint(j), scene->getPoint(k));
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addObject(std::move(newObject));
    }
  }
  
  scene->addLight(std::make_unique<Light>(1, 1, 1, currentColor));
  // All Stanford objs are centered at (0,0,0) so set the eye behind the object
  scene->setEye({0, 0, 10});
  std::cout << "Scanned " << scene->getNumObjects() << " objects" << std::endl;

  return scene;
}

std::unique_ptr<Scene> readDataFromStream(std::istream& in) {
  std::string line;
  std::getline(in, line);
  std::vector<std::string> lineInfo = split(line, ' ');

  if (lineInfo.size() != 4) {
    std::cerr << "Supplied PNG info doesn't have the correct number of arguments. Expected 4. Got "
         << lineInfo.size()
         << '.'
         << std::endl;
    return nullptr;
  } else if (lineInfo.at(0) != "png") {
    std::cerr << "Expected PNG image type. Got " << lineInfo.at(0) << '.' << std::endl;
    return nullptr;
  }

  int width = std::stoi(lineInfo.at(1));
  int height = std::stoi(lineInfo.at(2));
  std::string filename = lineInfo.at(3);
  std::unique_ptr<Scene> scene = std::make_unique<Scene>(width, height, filename);
  std::unique_ptr<Material> currentMaterial = std::make_unique<Material>(Vector3D(0,0,0), Vector3D(0,0,0), 1.458, 0.0);
  RGBAColor currentColor(1, 1, 1, 1);

  for (; std::getline(in, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "sphere") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      double r = std::stod(lineInfo.at(4));
      std::unique_ptr<Sphere> newObject = std::make_unique<Sphere>(x, y, z, r);
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addObject(std::move(newObject));
    } else if (keyword == "sun") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      std::unique_ptr<Light> newLight = std::make_unique<Light>(x, y, z, currentColor);
      scene->addLight(std::move(newLight));
    } else if (keyword == "color") {
      double r = std::stod(lineInfo.at(1));
      double g = std::stod(lineInfo.at(2));
      double b = std::stod(lineInfo.at(3));
      currentColor = RGBAColor(r, g, b, 1);
    } else if (keyword == "plane") {
      double A = std::stod(lineInfo.at(1));
      double B = std::stod(lineInfo.at(2));
      double C = std::stod(lineInfo.at(3));
      double D = std::stod(lineInfo.at(4));
      std::unique_ptr<Plane> newObject = std::make_unique<Plane>(A, B, C, D);
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addPlane(std::move(newObject));
    } else if (keyword == "bulb") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      std::unique_ptr<Bulb> newBulb = std::make_unique<Bulb>(x, y, z, currentColor);
      scene->addBulb(std::move(newBulb));
    } else if (keyword == "xyz") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      scene->addPoint(x, y, z);
    } else if (keyword == "trif") {
      int i = std::stoi(lineInfo.at(1)) - 1;
      int j = std::stoi(lineInfo.at(2)) - 1;
      int k = std::stoi(lineInfo.at(3)) - 1;
      std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(scene->getPoint(i), scene->getPoint(j), scene->getPoint(k));
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addObject(std::move(newObject));
    } else if (keyword == "expose") {
      double exposure = std::stod(lineInfo.at(1));
      scene->setExposure(exposure);
    } else if (keyword == "shininess") {
      double Sr = std::stod(lineInfo.at(1));
      double Sg = Sr;
      double Sb = Sr;
      if (lineInfo.size() > 2) {
        Sg = std::stod(lineInfo.at(2));
        Sb = std::stod(lineInfo.at(3));
      }
      currentMaterial->shine = Vector3D(Sr, Sg, Sb);
    } else if (keyword == "bounces") {
      double d = std::stoi(lineInfo.at(1));
      scene->setMaxBounces(d);
    } else if (keyword == "transparency") {
      double Tr = std::stod(lineInfo.at(1));
      double Tg = Tr;
      double Tb = Tr;
      if (lineInfo.size() > 2) {
        Tg = std::stod(lineInfo.at(2));
        Tb = std::stod(lineInfo.at(3));
      }
      currentMaterial->transparency = Vector3D(Tr, Tg, Tb);
    } else if (keyword == "aa") {
      int n = std::stoi(lineInfo.at(1));
      scene->setNumRays(n);
    } else if (keyword == "roughness") {
      double roughness = std::stod(lineInfo.at(1));
      currentMaterial->roughness = roughness;
      currentMaterial->roughnessDistribution = std::normal_distribution<>(0, roughness);
    } else if (keyword == "eye") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      scene->setEye(Vector3D(x, y, z));
    } else if (keyword == "forward") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      scene->setForward(Vector3D(x, y, z));
    } else if (keyword == "up") {
      double x = std::stod(lineInfo.at(1));
      double y = std::stod(lineInfo.at(2));
      double z = std::stod(lineInfo.at(3));
      scene->setUp(Vector3D(x, y, z));
    } else if (keyword == "fisheye") {
      scene->enableFisheye();
    } else if (keyword == "ior") {
      double ior = std::stod(lineInfo.at(1));
      currentMaterial->indexOfRefraction = ior;
    } else if (keyword == "gi") {
      int gi = std::stoi(lineInfo.at(1));
      scene->setGlobalIllumination(gi);
    } else if (keyword == "dof") {
      double focus = std::stod(lineInfo.at(1));
      double lens = std::stod(lineInfo.at(2));
      scene->setFocus(focus);
      scene->setLens(lens);
    } else if (keyword == "glass") {
      currentMaterial = std::make_unique<Glass>();
    } else if (keyword == "none") {
      currentMaterial = std::make_unique<Material>(Vector3D(0,0,0), Vector3D(0,0,0), 1.458, 0.0);
    }
  }

  return scene;
}

std::unique_ptr<Scene> readFromFile(const std::string& filename) {
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "Couldn't open file " << filename << std::endl;
    return nullptr;
  }
  if (ends_with(filename, ".txt")) {
    return readDataFromStream(infile);
  } else if (ends_with(filename, ".obj")) {
    std::unique_ptr<Scene> scene = readOBJ(infile);
    std::string pngFilename = filename.substr(0, filename.size() - 3) + "png";
    scene->setFilename(pngFilename);
    return scene;
  } else {
    std::cerr << "Unrecognized file format " << filename << std::endl;
    return nullptr;
  }
}