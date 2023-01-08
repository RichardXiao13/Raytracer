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

bool loadOBJ(
  const Vector3D &center,
  float scale,
  const std::string &filename,
  std::unique_ptr<Scene> &scene,
  const RGBAColor &color,
  const std::shared_ptr<Material> material)
{
  std::ifstream infile(filename);
  if (!infile) {
    std::cerr << "Couldn't open file " << filename << std::endl;
    return false;
  }

  std::vector<Vector3D> points;
  std::vector<Vector3D> normals;
  std::vector<std::string> lineInfo;
  Box extent;

  for (std::string line; std::getline(infile, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "v") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      extent.shrink(Vector3D(x, y, z));
      extent.expand(Vector3D(x, y, z));
      points.emplace_back(x, y, z);
    } else if (keyword == "vn") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      normals.emplace_back(x, y, z);
    }
  }

  // We want to center the object at 'center', so shift each point by 'center - (extent.maxPoint + extent.minPoint / 2)'
  // Also want to scale the obj down to a max of 1 along the biggest dimension to normalize,
  // so divide each point by 'max(extent.maxPoint - extent.minPoint)'
  // Then scale up by the 'scale' factor
  Vector3D shift = center - (extent.maxPoint + extent.minPoint) / 2;
  float scaleFactor = scale / maxDimension(extent.maxPoint - extent.minPoint);
  int numObjects = 0;
  infile = std::ifstream(filename);

  for (std::string line; std::getline(infile, line);) {
    lineInfo = split(line, ' ');
    if (lineInfo.size() == 0) {
      continue;
    }

    std::string keyword = lineInfo.at(0);

    if (keyword == "f") {
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

        Vector3D v1 = scaleFactor * points.at(i) + shift;
        Vector3D v2 = scaleFactor * points.at(j) + shift;
        Vector3D v3 = scaleFactor * points.at(k) + shift;

        std::unique_ptr<Triangle> newObject = std::make_unique<Triangle>(v1, v2, v3, color, material);
        if (vertex1.size() == 3) {
          newObject->n1 = normals.at(vertex1.at(2) - 1);
          newObject->n2 = normals.at(vertex2.at(2) - 1);
          newObject->n3 = normals.at(vertex3.at(2) - 1);
        }
        scene->addObject(std::move(newObject));
        ++numObjects;
      }
    }
  }
  
  std::cout << "Scanned " << points.size() << " points and " << numObjects << " objects" << std::endl;
  return true;
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
  std::shared_ptr<PNG> currentTexture = nullptr;

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
      std::unique_ptr<Sphere> newObject = std::make_unique<Sphere>(x, y, z, r, currentColor, currentMaterial, currentTexture);
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
      currentColor = RGBAColor(0,0,0,0);
      currentObjectType = ObjectType::Refractive;
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 1.5f, 1.0f, 1.0f, 0.0f, 0.0f, MaterialType::Glass);
    } else if (keyword == "plastic") {
      currentObjectType = ObjectType::Diffuse;
      currentMaterial = std::make_shared<Material>(0.5f, 0.5f, 1.3f, 1.0f, 0.0f, 0.0f, 0.1f, MaterialType::Plastic);
    } else if (keyword == "none") {
      currentObjectType = ObjectType::Diffuse;
      currentTexture = nullptr;
      currentMaterial = std::make_shared<Material>();
    } else if (keyword == "copper") {
      currentObjectType = ObjectType::Metal;
      // currentColor = RGBAColor(0.95597f, 0.63760f, 0.53948f);
      // https://en.wikipedia.org/wiki/Copper_(color)
      // Copper
      // currentColor = RGBAColor(0.4793201831f, 0.1714411007f, 0.03310476657f);
      // Pale Copper
      currentColor = RGBAColor(0.7011018919f, 0.2541520943f, 0.1356333297f);
      // Copper Red
      // currentColor = RGBAColor(0.5972017884f, 0.152926152f, 0.08228270713f);
      // Copper Penny
      // currentColor = RGBAColor(0.4178850708f, 0.1589608351f, 0.1412632911f);
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 0.23883f, 0.9553f, 0.0f, 3.415658f, 0.01f, MaterialType::Metal);
    } else if (keyword == "gold") {
      currentObjectType = ObjectType::Metal;
      // https://en.wikipedia.org/wiki/Gold_(color)
      // Gold (golden)
      // currentColor = RGBAColor(1.0f, 0.6795424696330938f, 0.0f);
      // Metallic Gold
      currentColor = RGBAColor(0.6583748172794485f, 0.4286904966139066f, 0.0382043715953465f);
      currentMaterial = std::make_shared<Material>(0.0f, 1.0f, 0.18104f, 0.99f, 0.0f, 3.068099f, 0.01f, MaterialType::Metal);
    } else if (keyword == "mirror") {
      currentColor = RGBAColor(0,0,0,0);
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
      const std::string &textureName = lineInfo.at(1);
      if (textures.find(textureName) != textures.end()) {
        currentTexture = textures[textureName];
      } else {
        currentTexture = std::make_shared<PNG>(textureName);
        textures[textureName] = currentTexture;
      }
    } else if (keyword == "obj") {
      float x = std::stof(lineInfo.at(1));
      float y = std::stof(lineInfo.at(2));
      float z = std::stof(lineInfo.at(3));
      float s = std::stof(lineInfo.at(4));
      bool error = loadOBJ(Vector3D(x, y, z), s, lineInfo.at(5), scene, currentColor, currentMaterial);
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
  } else {
    std::cerr << "Unrecognized file format " << filename << std::endl;
    return nullptr;
  }
}
