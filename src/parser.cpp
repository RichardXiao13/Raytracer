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
#include "materials/Plastic.h"

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
  std::unique_ptr<Scene> scene = std::make_unique<Scene>(1024, 1024, "");
  std::vector<Vector3D> points;
  std::unique_ptr<Material> currentMaterial = std::make_unique<Material>(Vector3D(0,0,0), Vector3D(0,0,0), 1.458, 0.0);
  RGBAColor currentColor(1, 1, 1, 1);
  float minX = INF_D;
  float minY = INF_D;
  float minZ = INF_D;
  float maxX = -INF_D;
  float maxY = -INF_D;
  float maxZ = -INF_D;

  for (; std::getline(in, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "v") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      minX = std::min(x, minX);
      minY = std::min(y, minY);
      minZ = std::min(z, minZ);
      maxX = std::max(x, maxX);
      maxY = std::max(y, maxY);
      maxZ = std::max(z, maxZ);
      points.emplace_back(x, y, z);
    } else if (keyword == "f") {
      // didn't deal with negative indices yet
      int numPoints = points.size();
      int i = std::stoi(lineInfo.at(1)) - 1;
      int j;
      int k;
      for (size_t idx = 2; idx < lineInfo.size() - 1; ++idx) {
        j = std::stoi(lineInfo.at(idx)) - 1;
        k = std::stoi(lineInfo.at(idx + 1)) - 1;
        // Try skipping invalid indices?
        if (i >= numPoints || j >= numPoints || k >= numPoints) {
          std::cout << "Indices out of range: " << i << ' ' << j << ' ' << k << std::endl;
          continue;
        }
        std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(points.at(i), points.at(j), points.at(k));
        std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
        newObject->setColor(currentColor);
        newObject->setMaterial(std::move(material));
        scene->addObject(std::move(newObject));
      }
    } else {
      // std::cout << "Unknown keyword " << keyword << std::endl;
    }
  }
  
  scene->addLight(std::make_unique<Light>(1, 1, 1, currentColor));
  // All Stanford objs are centered at (0,0,0)
  // so set the eye behind the object and centered based on object's width and height
  std::cout << maxX - minX << ' ' <<  maxY - minY << ' ' << maxZ << std::endl;
  float zExtent = std::max(maxZ,std::max(maxX-minX, maxY-minY));
  if (abs(zExtent - maxZ) < 1e-4) {
    zExtent *= 1.2;
  } else {
    zExtent = maxZ + zExtent/2;
  }
  scene->setEye({(minX + maxX) / 2, (minY + maxY) / 2, zExtent});
  scene->setNumRays(20);
  std::cout << "Scanned " << points.size() << " points and " << scene->getNumObjects() << " objects" << std::endl;

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
  std::vector<Vector3D> points;
  std::unique_ptr<Material> currentMaterial = std::make_unique<Material>(Vector3D(0,0,0), Vector3D(0,0,0), 1.458, 0.0);
  RGBAColor currentColor(1, 1, 1, 1);

  for (; std::getline(in, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "sphere") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      float r = std::stof(lineInfo.at(4));
      std::unique_ptr<Sphere> newObject = std::make_unique<Sphere>(x, y, z, r);
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addObject(std::move(newObject));
    } else if (keyword == "sun") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      std::unique_ptr<Light> newLight = std::make_unique<Light>(x, y, z, currentColor);
      scene->addLight(std::move(newLight));
    } else if (keyword == "color") {
      float r = std::stof(lineInfo.at(1));
      float g = std::stof(lineInfo.at(2));
      float b = std::stof(lineInfo.at(3));
      currentColor = RGBAColor(r, g, b, 1);
    } else if (keyword == "plane") {
      float A = std::stof(lineInfo.at(1));
      float B = std::stof(lineInfo.at(2));
      float C = std::stof(lineInfo.at(3));
      float D = std::stof(lineInfo.at(4));
      std::unique_ptr<Plane> newObject = std::make_unique<Plane>(A, B, C, D);
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addPlane(std::move(newObject));
    } else if (keyword == "bulb") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      std::unique_ptr<Bulb> newBulb = std::make_unique<Bulb>(x, y, z, currentColor);
      scene->addBulb(std::move(newBulb));
    } else if (keyword == "xyz") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      points.emplace_back(x, y, z);
    } else if (keyword == "trif") {
      int i = std::stoi(lineInfo.at(1)) - 1;
      int j = std::stoi(lineInfo.at(2)) - 1;
      int k = std::stoi(lineInfo.at(3)) - 1;
      if (i < 0) {
        i += points.size() + 1;
      }
      if (j < 0) {
        j += points.size() + 1;
      }
      if (k < 0) {
        k += points.size() + 1;
      }
      std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(points.at(i), points.at(j), points.at(k));
      std::unique_ptr<Material> material = std::make_unique<Material>(*currentMaterial);
      newObject->setColor(currentColor);
      newObject->setMaterial(std::move(material));
      scene->addObject(std::move(newObject));
    } else if (keyword == "expose") {
      float exposure = std::stof(lineInfo.at(1));
      scene->setExposure(exposure);
    } else if (keyword == "shininess") {
      float Sr = std::stof(lineInfo.at(1));
      float Sg = Sr;
      float Sb = Sr;
      if (lineInfo.size() > 2) {
        Sg = std::stof(lineInfo.at(2));
        Sb = std::stof(lineInfo.at(3));
      }
      currentMaterial->shine = Vector3D(Sr, Sg, Sb);
    } else if (keyword == "bounces") {
      float d = std::stoi(lineInfo.at(1));
      scene->setMaxBounces(d);
    } else if (keyword == "transparency") {
      float Tr = std::stof(lineInfo.at(1));
      float Tg = Tr;
      float Tb = Tr;
      if (lineInfo.size() > 2) {
        Tg = std::stof(lineInfo.at(2));
        Tb = std::stof(lineInfo.at(3));
      }
      currentMaterial->transparency = Vector3D(Tr, Tg, Tb);
    } else if (keyword == "aa") {
      int n = std::stoi(lineInfo.at(1));
      scene->setNumRays(n);
    } else if (keyword == "roughness") {
      float roughness = std::stof(lineInfo.at(1));
      currentMaterial->roughness = roughness;
      currentMaterial->roughnessDistribution = std::normal_distribution<>(0, roughness);
    } else if (keyword == "eye") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      scene->setEye(Vector3D(x, y, z));
    } else if (keyword == "forward") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      scene->setForward(Vector3D(x, y, z));
    } else if (keyword == "up") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      scene->setUp(Vector3D(x, y, z));
    } else if (keyword == "fisheye") {
      scene->enableFisheye();
    } else if (keyword == "ior") {
      float ior = std::stof(lineInfo.at(1));
      currentMaterial->indexOfRefraction = ior;
    } else if (keyword == "gi") {
      int gi = std::stoi(lineInfo.at(1));
      scene->setGlobalIllumination(gi);
    } else if (keyword == "dof") {
      float focus = std::stof(lineInfo.at(1));
      float lens = std::stof(lineInfo.at(2));
      scene->setFocus(focus);
      scene->setLens(lens);
    } else if (keyword == "glass") {
      currentMaterial = std::make_unique<Glass>();
    } else if (keyword == "plastic") {
      currentMaterial = std::make_unique<Plastic>();
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
