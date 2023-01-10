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
    if (line.rfind(TagNames[i]) == 0)
      return static_cast<Tag>(i);
  }
  return Tag::Unknown;
}

bool findOptions(const std::string &line, std::vector<std::string> *options) {
  // don't add here, otherwise overflow!!
  size_t start = line.find("options={") + 9;
  if (start == std::string::npos)
    return false;

  size_t end = line.find("}", start);
  if (end == std::string::npos)
    return false;
  
  // skip over "options={"
  std::string optionsString = line.substr(start, end - start);
  *options = split(optionsString, ';');
  return true;
}

bool stringTupleToVector3D(const std::string &string, Vector3D *vector) {
  size_t end = string.size();
  if (string[0] != '(' || string[end - 1] != ')')
    return false;
  
  std::vector<std::string> values = split(string.substr(1, end - 1), ',');
  if (values.size() != 3)
    return false;

  vector->x = std::stof(trim_copy(values[0]));
  vector->y = std::stof(trim_copy(values[1]));
  vector->z = std::stof(trim_copy(values[2]));
  return true;
}

bool findType(const std::string &line, std::string *type) {
  // don't add here, otherwise overflow!!
  size_t start = line.find("type=\"") + 6;
  if (start == std::string::npos)
    return false;

  size_t end = line.find('"', start);
  if (end == std::string::npos)
    return false;
  
  *type = line.substr(start, end - start);
  return true;
}

bool findName(const std::string &line, std::string *type) {
  // don't add here, otherwise overflow!!
  size_t start = line.find("name=\"") + 6;
  if (start == std::string::npos)
    return false;

  size_t end = line.find('"', start);
  if (end == std::string::npos)
    return false;
  
  *type = line.substr(start, end - start);
  return true;
}

bool findPath(const std::string &line, std::string *type) {
  // don't add here, otherwise overflow!!
  size_t start = line.find("path=\"") + 6;
  if (start == std::string::npos)
    return false;

  size_t end = line.find('"', start);
  if (end == std::string::npos)
    return false;
  
  *type = line.substr(start, end - start);
  return true;
}

bool parseDistantLightOptions(const std::vector<std::string> &options, Light **light) {
  RGBAColor color;
  Vector3D direction;
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return false;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return false;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    } else if (key == "direction") {
      if (stringTupleToVector3D(value, &direction) == false)
        return false;
    } else {
      return false;
    }
  }
  // @TODO make sure direction and color are both set, otherwise return false
  *light = new DistantLight(direction, color);
  return true;
}

bool parsePointLightOptions(const std::vector<std::string> &options, Light **light) {
  RGBAColor color;
  Vector3D center;
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return false;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return false;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    } else if (key == "center") {
      if (stringTupleToVector3D(value, &center) == false)
        return false;
    } else {
      return false;
    }
  }
  // @TODO make sure center and color are both set, otherwise return false
  *light = new PointLight(center, color);
  return true;
}

bool parseSphereOptions(
  const std::vector<std::string> &options,
  Sphere **sphere,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{
  Vector3D center;
  float radius = 1;
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return false;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return false;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    } else if (key == "center") {
      if (stringTupleToVector3D(value, &center) == false)
        return false;
    } else if (key == "radius") {
      radius = std::stof(value);
    } else {
      return false;
    }
  }

  // @TODO make sure center, color, and radius are set, otherwise return false
  *sphere = new Sphere(center, radius, color, material, texture);
  return true;
}

bool parseTriangleOptions(
  const std::vector<std::string> &options,
  Triangle **triangle,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{
  Vector3D p1, p2, p3;
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return false;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return false;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    } else if (key == "p1") {
      if (stringTupleToVector3D(value, &p1) == false)
        return false;
    } else if (key == "p2") {
      if (stringTupleToVector3D(value, &p2) == false)
        return false;
    } else if (key == "p3") {
      if (stringTupleToVector3D(value, &p3) == false)
        return false;
    } else {
      return false;
    }
  }

  // @TODO make sure p1, p2, and p3 are set, otherwise return false
  *triangle = new Triangle(p1, p2, p3, color, material, texture);
  return true;
}

bool parsePlaneOptions(
  const std::vector<std::string> &options,
  Plane **plane,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
)
{
  Vector3D normal;
  float D;
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return false;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return false;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    } else if (key == "normal") {
      if (stringTupleToVector3D(value, &normal) == false)
        return false;
    } else if (key == "D") {
      D = std::stof(value);
    } else {
      return false;
    }
  }

  // @TODO make sure center, color, and radius are set, otherwise return false
  *plane = new Plane(normal, D, color, material, texture);
  return true;
}

ParserTree::ParserTree(std::ifstream &filestream) {
  // build root node before building everything else
  // root will contain Scene tag information
  std::string line;
  std::getline(filestream, line);

  size_t start = line.rfind('<');
  size_t end = line.rfind('>');
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
  std::vector<std::string> options;
  if (findOptions(node->content, &options) == false)
    return;
  
  SceneOptions sceneOptions;

  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);
    
    if (key == "bias") {
      sceneOptions.bias = std::stof(value);
    } else if (key == "exposure") {
      sceneOptions.exposure = std::stof(value);
    } else if (key == "maxBounces") {
      sceneOptions.maxBounces = std::stoi(value);
    } else if (key == "numRays") {
      sceneOptions.numRays = std::stoi(value);
    } else if (key == "fisheye") {
      sceneOptions.fisheye = std::stoi(value);
    } else if (key == "focus") {
      sceneOptions.focus = std::stof(value);
    } else if (key == "lens") {
      sceneOptions.lens = std::stof(value);
    } else if (key == "width") {
      scene->setWidth(std::stoi(value));
    } else if (key == "height") {
      scene->setHeight(std::stoi(value));
    } else if (key == "filename") {
      scene->setFilename(value);
    } else {
      // unknown scene option
      return;
    }
  }

  scene->options = sceneOptions;
  for (Node *child : node->children) {
    parseNode(child, scene);
  }
}

void ParserTree::parseCameraNode(Node *node, Scene *scene) {
  std::vector<std::string> options;
  if (findOptions(node->content, &options) == false)
    return;

  Camera camera;
  
  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (option == "eye") {
      Vector3D eye;
      if (stringTupleToVector3D(value, &eye) == false)
        return;
      camera.setEye(eye);
    } else if (option == "forward") {
      Vector3D forward;
      if (stringTupleToVector3D(value, &forward) == false)
        return;
      camera.setForward(forward);
    } else if (option == "up") {
      Vector3D up;
      if (stringTupleToVector3D(value, &up) == false)
        return;
      camera.setForward(up);
    } else {
      // unknown camera option
      return;
    }
  }

  scene->camera = camera;
}

void ParserTree::parseLightNode(Node *node, Scene *scene) {
  std::vector<std::string> options;
  if (findOptions(node->content, &options) == false)
    return;
  
  std::string type;
  if (findType(node->content, &type) == false)
    return;

  Light *light;

  if (type == "distant") {
    if (parseDistantLightOptions(options, &light) == false)
      return;
  } else if (type == "point") {
    if (parsePointLightOptions(options, &light) == false)
      return;
  } else {
    // unknown light type
    return;
  }

  // @TODO add environment light
  scene->addLight(light);
}

std::shared_ptr<Material> ParserTree::parseMaterialNode(Node *node, RGBAColor *color, ObjectType *type) {
  std::string name;
  findName(node->content, &name);
  if (name == "glass") {
    *color = RGBAColor(0,0,0,0);
    *type = ObjectType::Refractive;
    return std::make_shared<Material>(0.0f, 1.0f, 1.5f, 1.0f, 1.0f, 0.0f, 0.0f, MaterialType::Glass);
  } else if (name == "plastic") {
      *type = ObjectType::Diffuse;
      return std::make_shared<Material>(0.5f, 0.5f, 1.3f, 1.0f, 0.0f, 0.0f, 0.1f, MaterialType::Plastic);
  } else if (name == "copper") {
    *type = ObjectType::Metal;
    // *color = RGBAColor(0.95597f, 0.63760f, 0.53948f);
    // https://en.wikipedia.org/wiki/Copper_(color)
    // Copper
    // *color = RGBAColor(0.4793201831f, 0.1714411007f, 0.03310476657f);
    // Pale Copper
    *color = RGBAColor(0.7011018919f, 0.2541520943f, 0.1356333297f);
    // Copper Red
    // *color = RGBAColor(0.5972017884f, 0.152926152f, 0.08228270713f);
    // Copper Penny
    // *color = RGBAColor(0.4178850708f, 0.1589608351f, 0.1412632911f);
    return std::make_shared<Material>(0.0f, 1.0f, 0.23883f, 0.9553f, 0.0f, 3.415658f, 0.01f, MaterialType::Metal);
  } else if (name == "gold") {
    *type = ObjectType::Metal;
    // https://en.wikipedia.org/wiki/Gold_(color)
    // Gold (golden)
    // *color = RGBAColor(1.0f, 0.6795424696330938f, 0.0f);
    // Metallic Gold
    *color = RGBAColor(0.6583748172794485f, 0.4286904966139066f, 0.0382043715953465f);
    return std::make_shared<Material>(0.0f, 1.0f, 0.18104f, 0.99f, 0.0f, 3.068099f, 0.01f, MaterialType::Metal);
  } else if (name == "mirror") {
    *color = RGBAColor(0,0,0,0);
    *type = ObjectType::Reflective;
    return std::make_shared<Material>(0.0f, 1.0f, 0.0f, 0.9f, 0.0f, 0.0f, 0.0f, MaterialType::Mirror);
  }
  return nullptr;
}

std::shared_ptr<PNG> ParserTree::parseTextureNode(Node *node) {
  std::string path;
  if (findPath(node->content, &path) == false)
    return nullptr;
  return std::make_shared<PNG>(path);
}

void ParserTree::parseShapeNode(Node *node, Scene *scene) {
  std::vector<std::string> options;
  if (findOptions(node->content, &options) == false)
    return;
  
  std::string type;
  if (findType(node->content, &type) == false)
    return;

  RGBAColor color(1,1,1,1);
  ObjectType objectType = ObjectType::Diffuse;
  std::shared_ptr<Material> material = std::make_unique<Material>();
  std::shared_ptr<PNG> texture = nullptr;
  for (Node *child : node->children) {
    if (child->type == Tag::Material) {
      material = parseMaterialNode(child, &color, &objectType);
    } else if (child->type == Tag::Texture) {
      texture = parseTextureNode(child);
    }
  }


  if (type == "sphere") {
    Sphere *sphere;
    if(parseSphereOptions(options, &sphere, color, objectType, material, texture) == false)
      return;
    scene->addObject(std::unique_ptr<Object>(sphere));
  } else if (type == "triangle") {
    Triangle *triangle;
    if (parseTriangleOptions(options, &triangle, color, objectType, material, texture) == false)
      return;
    if (dot(scene->camera.forward, triangle->centroid - scene->camera.eye) > 0
        && dot(scene->camera.forward, triangle->normal) > 0) {
        triangle->normal *= -1.0f;
      }
      triangle->n1 = triangle->normal;
      triangle->n2 = triangle->normal;
      triangle->n3 = triangle->normal;
    scene->addObject(std::unique_ptr<Triangle>(triangle));
  } else if (type == "plane") {
    Plane *plane;
    if (parsePlaneOptions(options, &plane, color, objectType, material, texture) == false)
      return;
    scene->addPlane(std::unique_ptr<Plane>(plane));
  } else {
    // unknown shape
    return;
  }
}

void ParserTree::parseWavefrontNode(Node *node, Scene *scene) {
  std::vector<std::string> options;
  if (findOptions(node->content, &options) == false)
    return;
  
  std::string path;
  if (findPath(node->content, &path) == false)
    return;

  RGBAColor color(1,1,1,1);
  ObjectType objectType = ObjectType::Diffuse;
  std::shared_ptr<Material> material = std::make_unique<Material>();
  for (Node *child : node->children) {
    if (child->type == Tag::Material) {
      material = parseMaterialNode(child, &color, &objectType);
    }
  }

  Vector3D center;
  float scale = 1;

  for (const std::string &option : options) {
    std::vector<std::string> keyValue = split(option, ':');
    if (keyValue.size() != 2)
      return;
    
    std::string key = trim_copy(keyValue[0]);
    std::string value = trim_copy(keyValue[1]);

    if (key == "center") {
      if (stringTupleToVector3D(value, &center) == false)
        return;
    } else if (key == "scale") {
      scale = std::stof(value);
    } else if (key == "color") {
      Vector3D temp;
      if (stringTupleToVector3D(value, &temp) == false)
        return;
      color.r = temp.x;
      color.g = temp.y;
      color.b = temp.z;
    }
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
    }

    size_t start = line.rfind('<');
    size_t end = line.rfind('>');
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
  std::string line;
  Scene *scene = new Scene();
  ParserTree tree(filestream);
  return tree.parseIntoScene();
}
