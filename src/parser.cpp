#include "macros.h"
#include "parser.h"
#include "raytracer.h"
#include "Objects.h"
#include "Material.h"
#include "Profiler.h"

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

std::vector<int> parseOBJPoint(const std::string &s) {
  std::vector<int> indices;
  std::istringstream iss(s);
  std::string item;
  while (std::getline(iss, item, '/')) {
    if (!item.empty()) {
      indices.push_back(stoi(item));
    } else {
      indices.push_back(-1);
    }
  }
  return indices;
}

std::unique_ptr<Scene> readOBJ(std::istream& in) {
  std::string line;
  std::getline(in, line);
  std::vector<std::string> lineInfo = split(line, ' ');

  // Will set scene filename later
  std::unique_ptr<Scene> scene = std::make_unique<Scene>(1024, 1024, "");
  std::vector<Vector3D> points;
  std::vector<Vector3D> normals;
  std::shared_ptr<Material> currentMaterial = std::make_shared<Material>();
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
    } else if (keyword == "vn") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      normals.emplace_back(x, y, z);
    } else if (keyword == "f") {
      // didn't deal with negative indices yet
      int numPoints = points.size();
      std::vector<int> vertex1 = parseOBJPoint(lineInfo.at(1));
      int i = vertex1.at(0) - 1;
      int j;
      int k;
      for (size_t idx = 2; idx < lineInfo.size() - 1; ++idx) {
        std::vector<int> vertex2 = parseOBJPoint(lineInfo.at(idx));
        std::vector<int> vertex3 = parseOBJPoint(lineInfo.at(idx + 1));
        j = vertex2.at(0) - 1;
        k = vertex3.at(0) - 1;
        // Try skipping invalid indices?
        if (i >= numPoints || j >= numPoints || k >= numPoints) {
          std::cout << "Indices out of range: " << i << ' ' << j << ' ' << k << std::endl;
          continue;
        }
        std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(points.at(i), points.at(j), points.at(k), currentColor, currentMaterial);
        if (vertex1.size() == 3) {
          newObject->n1 = normals.at(vertex1.at(2) - 1);
          newObject->n2 = normals.at(vertex2.at(2) - 1);
          newObject->n3 = normals.at(vertex3.at(2) - 1);
        }
        scene->addObject(std::move(newObject));
      }
    } else {
      // std::cout << "Unknown keyword " << keyword << std::endl;
    }
  }
  
  scene->addLight(new DistantLight(0.4, 0.4, 0.4, currentColor));
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
  scene->setNumRays(200);
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
  std::shared_ptr<Material> currentMaterial = std::make_shared<Material>();
  RGBAColor currentColor(1, 1, 1, 1);
  ObjectType currentObjectType = ObjectType::Diffuse;
  std::unordered_map<std::string, std::shared_ptr<PNG>> textures;

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
      std::unique_ptr<Sphere> newObject = std::make_unique<Sphere>(x, y, z, r, currentColor, currentMaterial);
      newObject->type = currentObjectType;
      scene->addObject(std::move(newObject));
    } else if (keyword == "sun") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      scene->addLight(new DistantLight(x, y, z, currentColor));
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
      std::unique_ptr<Plane> newObject = std::make_unique<Plane>(A, B, C, D, currentColor, currentMaterial);
      newObject->type = currentObjectType;
      scene->addPlane(std::move(newObject));
    } else if (keyword == "bulb") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      scene->addLight(new Bulb(x, y, z, currentColor));
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
      std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(points.at(i), points.at(j), points.at(k), currentColor, currentMaterial);
      newObject->type = currentObjectType;
      // Orient the normal if the normal faces with the forward vector and the object is in front of the camera
      // I think this works??
      if (dot(scene->forward, newObject->centroid - scene->eye) > 0 && dot(scene->forward, newObject->normal) > 0) {
        newObject->normal *= -1.0f;
      }
      newObject->n1 = newObject->normal;
      newObject->n2 = newObject->normal;
      newObject->n3 = newObject->normal;
      scene->addObject(std::move(newObject));
    } else if (keyword == "expose") {
      float exposure = std::stof(lineInfo.at(1));
      scene->setExposure(exposure);
    } else if (keyword == "bounces") {
      float d = std::stoi(lineInfo.at(1));
      scene->setMaxBounces(d);
    } else if (keyword == "aa") {
      int n = std::stoi(lineInfo.at(1));
      scene->setNumRays(n);
    } else if (keyword == "roughness") {
      float roughness = std::stof(lineInfo.at(1));
      currentMaterial->roughness = roughness;
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
      float eta = std::stof(lineInfo.at(1));
      currentMaterial->eta = eta;
    } else if (keyword == "gi") {
      int gi = std::stoi(lineInfo.at(1));
      scene->setGlobalIllumination(gi);
    } else if (keyword == "specular") {
      int specularRays = std::stoi(lineInfo.at(1));
      scene->setSpecularRays(specularRays);
    } else if (keyword == "dof") {
      float focus = std::stof(lineInfo.at(1));
      float lens = std::stof(lineInfo.at(2));
      scene->setFocus(focus);
      scene->setLens(lens);
    } else if (keyword == "glass") {
      currentObjectType = ObjectType::Refractive;
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 1.5f, 1.0f, 1.0f, 0.0f, 0.0f, MaterialType::Glass);
    } else if (keyword == "plastic") {
      currentObjectType = ObjectType::Diffuse;
      currentMaterial = std::make_shared<Material>(0.5f, 0.5f, 1.3f, 1.0f, 0.0f, 0.0f, 0.1f, MaterialType::Plastic);
    } else if (keyword == "none") {
      currentObjectType = ObjectType::Diffuse;
      currentMaterial = std::make_shared<Material>();
    } else if (keyword == "copper") {
      currentObjectType = ObjectType::Metal;
      currentColor = RGBAColor(0.95597f, 0.63760f, 0.53948f);
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 0.23883f, 0.9553f, 0.0f, 0.0447f, 0.01f, MaterialType::Metal);
    } else if (keyword == "mirror") {
      currentObjectType = ObjectType::Reflective;
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, MaterialType::Mirror);
    } else if (keyword == "diffuse") {
      currentObjectType = ObjectType::Diffuse;
      currentMaterial = std::make_shared<Material>();
    } else if (keyword == "refractive") {
      currentObjectType = ObjectType::Refractive;
    } else if (keyword == "reflective") {
      currentObjectType = ObjectType::Reflective;
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, MaterialType::Dialectric);
    } else if (keyword == "texture") {
      textures[lineInfo.at(1)] = std::make_shared<PNG>(lineInfo.at(1));
    }
  }

  return scene;
}

std::unique_ptr<Scene> readFromFile(const std::string& filename) {
  Profiler p(Funcs::SceneConstruction);

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
