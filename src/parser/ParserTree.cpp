#include "ParserTree.h"

// https://stackoverflow.com/questions/216823/how-to-trim-an-stdstring
// trim from start (in place)
inline void ltrim(std::string &s) {
  s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
    return !std::isspace(ch);
  }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
  s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
    return !std::isspace(ch);
  }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
  rtrim(s);
  ltrim(s);
}

// trim from start (copying)
static inline std::string ltrim_copy(std::string s) {
  ltrim(s);
  return s;
}

// trim from end (copying)
static inline std::string rtrim_copy(std::string s) {
  rtrim(s);
  return s;
}

// trim from both ends (copying)
static inline std::string trim_copy(std::string s) {
  trim(s);
  return s;
}

Tag getTagType(const std::string &line) {
  for (size_t i = 0; i < TagNames.size(); ++i) {
    if (line.find(TagNames[i]) == 0)
      return static_cast<Tag>(i);
  }
  return Tag::Unknown;
}

std::string parseKeyword(const std::string &line, const std::string &keyword) {
  size_t start = line.find(keyword + "=\"");
  if (start == std::string::npos)
    return "";

  start += keyword.size() + 2;
  size_t end = line.find('"', start);
  return line.substr(start, end - start);
}

std::unordered_map<std::string, std::string> parseOptions(const std::string &line) {
  size_t start = line.find("options={") + 9;
  size_t end = line.find('}', start);
  std::string optionsString = line.substr(start, end - start);
  std::vector<std::string> optionsList = split(optionsString, ';');

  std::unordered_map<std::string, std::string> options;

  // Parse each option and add the pairs to the options table
  std::for_each(optionsList.begin(), optionsList.end(),
  [&](const std::string &option) {
    std::vector<std::string> keyValuePair = split(option, ':');
    std::string key = trim_copy(keyValuePair[0]);
    std::string value = trim_copy(keyValuePair[1]);

    options.insert({ key, value });
  });

  return options;
}

Vector3D stringTupleToVector3D(const std::string &string) {
  size_t end = string.size();
  // string is in format like '(x, x, x)'
  std::vector<std::string> values = split(string.substr(1, end - 1), ',');

  Vector3D vector;
  vector.x = std::stof(trim_copy(values[0]));
  vector.y = std::stof(trim_copy(values[1]));
  vector.z = std::stof(trim_copy(values[2]));
  return vector;
}

template <typename T>
T getDefaultOptionOrApply(
  const std::unordered_map<std::string, std::string> &options,
  const std::string &key,
  std::function<T (const std::string &)> func,
  const T &value
)
{
  auto it = options.find(key);
  return (it != options.end()) ? func(it->second) : value;
};

DistantLight *parseDistantLightOptions(const std::unordered_map<std::string, std::string> &options) {
  Vector3D direction = getDefaultOptionOrApply<Vector3D>(options, "direction", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D temp      = getDefaultOptionOrApply<Vector3D>(options, "color", &stringTupleToVector3D, Vector3D(1, 1, 1));
  RGBAColor color(temp.x, temp.y, temp.z, 1);

  // @TODO make sure direction and color are both set, otherwise return false
  return new DistantLight(direction, color);
}

PointLight *parsePointLightOptions(const std::unordered_map<std::string, std::string> &options) {
  Vector3D center = getDefaultOptionOrApply<Vector3D>(options, "center", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D temp   = getDefaultOptionOrApply<Vector3D>(options, "color", &stringTupleToVector3D, Vector3D(1, 1, 1));
  RGBAColor color(temp.x, temp.y, temp.z, 1);

  // @TODO make sure center and color are both set, otherwise return false
  return new PointLight(center, color);
}

EnvironmentLight *parseEnvironmentLightOptions(const std::unordered_map<std::string, std::string> &options, const std::string &path, const Vector3D &center) {
  auto stof = [](const std::string &str) {
    return std::stof(str);
  };
  float radius = getDefaultOptionOrApply<float>(options, "radius", stof, 1.0f);

  if (path.empty()) {
    Vector3D temp = getDefaultOptionOrApply<Vector3D>(options, "color", &stringTupleToVector3D, Vector3D(1, 1, 1));
    RGBAColor color(temp.x, temp.y, temp.z, 1);
    return new EnvironmentLight(center, radius, color);
  }

  float scale = getDefaultOptionOrApply<float>(options, "scale", stof, 1.0f);
  return new EnvironmentLight(center, radius, scale, std::make_shared<PNG>(path));
}

Sphere *parseSphereOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{
  auto stof = [](const std::string &str) {
    return std::stof(str);
  };

  Vector3D center = getDefaultOptionOrApply<Vector3D>(options, "center", &stringTupleToVector3D, Vector3D(0, 0, 0));
  float radius    = getDefaultOptionOrApply<float>(options, "radius", stof, 1.0f);
  auto it = options.find("color");
  if (it != options.end()) {
    Vector3D temp = stringTupleToVector3D(it->second);
    color.r = temp.x;
    color.g = temp.y;
    color.b = temp.z;
  }

  // @TODO make sure center, color, and radius are set, otherwise return false
  return new Sphere(center, radius, color, material, texture);
}

Triangle *parseTriangleOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{

  Vector3D p1 = getDefaultOptionOrApply<Vector3D>(options, "p1", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D p2 = getDefaultOptionOrApply<Vector3D>(options, "p2", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D p3 = getDefaultOptionOrApply<Vector3D>(options, "p3", &stringTupleToVector3D, Vector3D(0, 0, 0));

  Vector3D t1 = getDefaultOptionOrApply<Vector3D>(options, "t1", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D t2 = getDefaultOptionOrApply<Vector3D>(options, "t2", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D t3 = getDefaultOptionOrApply<Vector3D>(options, "t3", &stringTupleToVector3D, Vector3D(0, 0, 0));

  auto it = options.find("color");
  if (it != options.end()) {
    Vector3D temp = stringTupleToVector3D(it->second);
    color.r = temp.x;
    color.g = temp.y;
    color.b = temp.z;
  }

  // @TODO make sure p1, p2, and p3 are set, otherwise return false
  return new Triangle(p1, p2, p3, color, material, t1, t2, t3, texture);
}

Plane *parsePlaneOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{
  auto stof = [](const std::string &str) {
    return std::stof(str);
  };

  Vector3D normal = getDefaultOptionOrApply<Vector3D>(options, "normal", &stringTupleToVector3D, Vector3D(0, 0, 0));
  float D         = getDefaultOptionOrApply<float>(options, "D", stof, 1.0f);
  auto it = options.find("color");
  if (it != options.end()) {
    Vector3D temp = stringTupleToVector3D(it->second);
    color.r = temp.x;
    color.g = temp.y;
    color.b = temp.z;
  }

  // @TODO make sure center, color, and radius are set, otherwise return false
  return new Plane(normal, D, color, material, texture);
}

ParserTree::ParserTree(std::ifstream &filestream) {
  // build root node before building everything else
  // root will contain Scene tag information
  std::string line;
  std::getline(filestream, line);

  size_t start = line.find('<');
  size_t end = line.find('>');
  std::string content = line.substr(start + 1, end - start - 1);

  root = new Node();
  root->type = getTagType(content);
  root->content = content;

  build(filestream, root);
}

std::unique_ptr<Scene> ParserTree::parseIntoScene() {
  Scene *scene = new Scene();
  parseNode(root, scene);
  return std::unique_ptr<Scene>(scene);
}

void ParserTree::parseSceneNode(Node *node, Scene *scene) {
  std::unordered_map<std::string, std::string> options = parseOptions(node->content);
  SceneOptions sceneOptions;

  auto stof = [](const std::string &str) {
    return std::stof(str);
  };

  auto stoi = [](const std::string &str) {
    return std::stoi(str);
  };

  sceneOptions.bias       = getDefaultOptionOrApply<float>(options, "bias", stof, 1e-4f);
  sceneOptions.exposure   = getDefaultOptionOrApply<float>(options, "exposure", stof, -1.0f);
  sceneOptions.maxBounces = getDefaultOptionOrApply<int>(options, "maxBounces", stoi, 4);
  sceneOptions.numRays    = getDefaultOptionOrApply<int>(options, "numRays", stoi, 1);
  sceneOptions.fisheye    = getDefaultOptionOrApply<int>(options, "fisheye", stoi, 0);
  sceneOptions.focus      = getDefaultOptionOrApply<float>(options, "focus", stof, -1.0f);
  sceneOptions.lens       = getDefaultOptionOrApply<float>(options, "lens", stoi, 0.0f);

  int width;
  int height;
  std::string filename;

  auto it = options.find("width");
  if (it == options.end()) {
    std::cerr << "Render image width not provided." << std::endl;
    exit(1);
  }
  width = std::stoi(it->second);
  it = options.find("height");
  if (it == options.end()) {
    std::cerr << "Render image height not provided." << std::endl;
    exit(1);
  }
  height = std::stoi(it->second);
  it = options.find("filename");
  if (it == options.end()) {
    std::cerr << "Render image file name not provided." << std::endl;
    exit(1);
  }
  filename = it->second;
  scene->setWidth(width);
  scene->setHeight(height);
  scene->setFilename(filename);

  scene->options = sceneOptions;

  // move lights to be parsed at the end so any environment lights have the correct world center
  int n = node->children.size();
  for (int i = 0; i < n; ++i) {
    if (node->children[i]->type == Tag::Light) {
      node->children.push_back(node->children[i]);
      node->children[i] = nullptr;
    }
  }
  for (Node *child : node->children) {
    if (child != nullptr)
    parseNode(child, scene);
  }
}

void ParserTree::parseCameraNode(Node *node, Scene *scene) {
  std::unordered_map<std::string, std::string> options = parseOptions(node->content);
  Camera camera;

  Vector3D eye     = getDefaultOptionOrApply<Vector3D>(options, "eye", &stringTupleToVector3D, Vector3D(0, 0, 0));
  Vector3D forward = getDefaultOptionOrApply<Vector3D>(options, "forward", &stringTupleToVector3D, Vector3D(0, 0, -1));
  Vector3D up      = getDefaultOptionOrApply<Vector3D>(options, "up", &stringTupleToVector3D, Vector3D(0, 1, 0));

  camera.setEye(eye);
  camera.setForward(forward);
  camera.setUp(up);

  scene->camera = camera;
}

void ParserTree::parseLightNode(Node *node, Scene *scene) {
  std::unordered_map<std::string, std::string> options = parseOptions(node->content);
  
  std::string type = parseKeyword(node->content, "type");

  Light *light;

  if (type == "distant") {
    light = parseDistantLightOptions(options);
  } else if (type == "point") {
    light = parsePointLightOptions(options);
  } else if (type == "environment") {
    std::string path = "";
    if (node->content.find("path") != std::string::npos)
      path = parseKeyword(node->content, "path");
    light = parseEnvironmentLightOptions(options, path, scene->worldCenter());
  } else {
    // unknown light type
    return;
  }

  scene->addLight(light);
}

std::shared_ptr<Material> ParserTree::parseMaterialNode(Node *node, RGBAColor *color, ObjectType *type) {
  std::string name = parseKeyword(node->content, "name");
  if (name.empty()) {
    std::unordered_map<std::string, std::string> options = parseOptions(node->content);

    auto stof = [](const std::string &str) {
      return std::stof(str);
    };

    auto stoi = [](const std::string &str) {
      return std::stoi(str);
    };

    auto toMaterialType = [](const std::string &str) {
      return NameToMaterialType.at(str);
    };

    float Kd          = getDefaultOptionOrApply<float>(options, "Kd", stof, 1e-4f);
    float Ks          = getDefaultOptionOrApply<float>(options, "Ks", stof, -1.0f);
    float eta         = getDefaultOptionOrApply<float>(options, "eta", stof, -1.0f);
    float Kr          = getDefaultOptionOrApply<float>(options, "Kr", stof, -1.0f);
    float Kt          = getDefaultOptionOrApply<float>(options, "Kt", stof, -1.0f);
    float Ka          = getDefaultOptionOrApply<float>(options, "Ka", stof, -1.0f);
    float roughness   = getDefaultOptionOrApply<float>(options, "roughness", stof, -1.0f);
    MaterialType type = getDefaultOptionOrApply<MaterialType>(options, "type", toMaterialType, MaterialType::Dialectric);
    return std::make_shared<Material>(Kd, Ks, eta, Kr, Kt, Ka, roughness, type);
  }

  if (name == "glass") {
    *color = RGBAColor(0,0,0,0);
    *type = ObjectType::Refractive;
    return NamedMaterials.at("glass");
  } else if (name == "plastic") {
      *type = ObjectType::Diffuse;
      return NamedMaterials.at("plastic");
  } else if (name.find("copper") != std::string::npos) {
    *type = ObjectType::Metal;
    *color = MaterialColors.at(name);
    return NamedMaterials.at("copper");
  } else if (name.find("gold") != std::string::npos) {
    *type = ObjectType::Metal;
    *color = MaterialColors.at(name);
    return NamedMaterials.at("gold");
  } else if (name == "mirror") {
    *color = RGBAColor(0,0,0,0);
    *type = ObjectType::Reflective;
    return NamedMaterials.at("mirror");
  } else {
    return NamedMaterials.at("default");
  }
}

std::shared_ptr<PNG> ParserTree::parseTextureNode(Node *node) {
  std::string path = parseKeyword(node->content, "path");
  return std::make_shared<PNG>(path);
}

void ParserTree::parseShapeNode(Node *node, Scene *scene) {
  std::unordered_map<std::string, std::string> options = parseOptions(node->content);
  std::string type = parseKeyword(node->content, "type");

  RGBAColor color(1,1,1,1);
  ObjectType objectType = ObjectType::Diffuse;
  std::shared_ptr<Material> material = NamedMaterials.at("default");
  std::shared_ptr<PNG> texture = nullptr;

  for (Node *child : node->children) {
    if (child->type == Tag::Material) {
      material = parseMaterialNode(child, &color, &objectType);
    } else if (child->type == Tag::Texture) {
      texture = parseTextureNode(child);
    }
  }

  if (type == "sphere") {
    Sphere *sphere = parseSphereOptions(options, color, objectType, material, texture);
    scene->addObject(std::unique_ptr<Object>(sphere));
  } else if (type == "triangle") {
    Triangle *triangle = parseTriangleOptions(options, color, objectType, material, texture);
    if (dot(scene->camera.forward, triangle->centroid - scene->camera.eye) > 0
        && dot(scene->camera.forward, triangle->normal) > 0) {
        triangle->normal *= -1.0f;
      }
      triangle->n1 = triangle->normal;
      triangle->n2 = triangle->normal;
      triangle->n3 = triangle->normal;
    scene->addObject(std::unique_ptr<Triangle>(triangle));
  } else if (type == "plane") {
    Plane *plane = parsePlaneOptions(options, color, objectType, material, texture);
    scene->addPlane(std::unique_ptr<Plane>(plane));
  } else {
    // unknown shape
    return;
  }
}

void ParserTree::parseWavefrontNode(Node *node, Scene *scene) {
  std::unordered_map<std::string, std::string> options = parseOptions(node->content);
  std::string path = parseKeyword(node->content, "path");

  RGBAColor color(1,1,1,1);
  ObjectType objectType = ObjectType::Diffuse;
  std::shared_ptr<Material> material = std::make_unique<Material>();
  for (Node *child : node->children) {
    if (child->type == Tag::Material) {
      material = parseMaterialNode(child, &color, &objectType);
    }
  }

  auto stof = [](const std::string &str) {
    return std::stof(str);
  };

  Vector3D center = getDefaultOptionOrApply<Vector3D>(options, "center", &stringTupleToVector3D, Vector3D(0, 0, 0));
  float scale     = getDefaultOptionOrApply<float>(options, "scale", stof, 1.0f);
  auto it = options.find("color");
  if (it != options.end()) {
    Vector3D temp = stringTupleToVector3D(it->second);
    color.r = temp.x;
    color.g = temp.y;
    color.b = temp.z;
  }

  loadOBJ(center, scale, path, scene, color, material);
}

void ParserTree::parseNode(Node *node, Scene *scene) {
  switch (node->type) {
    case Tag::Scene:
      parseSceneNode(node, scene);
      break;

    case Tag::Camera:
      parseCameraNode(node, scene);
      break;
    
    case Tag::Light:
      parseLightNode(node, scene);
      break;
    
    case Tag::Shape:
      parseShapeNode(node, scene);
      break;
    
    case Tag::Wavefront:
      parseWavefrontNode(node, scene);
      break;
    
    default:
      break;
  }
}

void ParserTree::print(Node *node) {
  std::cout << TagNames[static_cast<int>(node->type)] << " node contains " << node->content << std::endl;
  for (Node *child : node->children) {
    std::cout << TagNames[static_cast<int>(node->type)] << ' ' << child->content << std::endl;
  }

  std::cout << std::endl;

  for (Node *child : node->children) {
    print(child);
  }
}

void ParserTree::build(std::ifstream &filestream, Node *node) {
  // continue processing lines until matching closing tag is found
  while (true) {
    std::string line;
    while (line.empty()) {
      // get next tag to process
      if (!std::getline(filestream, line))
        return;
      trim(line);
    }

    size_t start = line.find('<');
    size_t end = line.find('>');
    std::string content = line.substr(start + 1, end - start - 1);
    Tag tag = getTagType(content);
    // if line is a closing tag and tag type matches current node, return
    if (content[0] == '/' && node->type == getTagType(content.substr(1)))
      return;

    Node *child = new Node();
    child->type = getTagType(content);
    // if line isn't a self closing tag, we need to build the child
    if (line[end - 1] != '/') {
      build(filestream, child);
    } else {
      content.pop_back();
    }
    child->content = content;
    
    node->children.push_back(child);
  }
}

std::unique_ptr<Scene> parseFile(std::ifstream &filestream) {
  ParserTree tree(filestream);
  return tree.parseIntoScene();
}
