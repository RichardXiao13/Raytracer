#pragma once

#include "macros.h"
#include "parser.h"

inline void ltrim(std::string &s);
inline void rtrim(std::string &s);
inline void trim(std::string &s);

static inline std::string ltrim_copy(std::string s);
static inline std::string rtrim_copy(std::string s);
static inline std::string trim_copy(std::string s);

Tag getTagType(const std::string &line);
std::string parseKeyword(const std::string &line, const std::string &keyword);
std::unordered_map<std::string, std::string> parseOptions(const std::string &line);
Vector3D stringTupleToVector3D(const std::string &string);

template <typename T>
T getDefaultOptionOrApply(
  const std::unordered_map<std::string, std::string> &options,
  const std::string &key,
  std::function<T (const std::string &)> func,
  const T &value
);

DistantLight *parseDistantLightOptions(const std::unordered_map<std::string, std::string> &options);
PointLight *parsePointLightOptions(const std::unordered_map<std::string, std::string> &options);

Sphere *parseSphereOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
);
Triangle *parseTriangleOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
);
Plane *parsePlaneOptions(
  const std::unordered_map<std::string, std::string> &options,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
);

class ParserTree {
public:
  struct Node {
    Tag type;
    std::string content;
    std::vector<Node*> children;
  };

  ParserTree(std::ifstream &filestream);
  std::unique_ptr<Scene> parseIntoScene();

  void print() {
    print(root);
  }

private:
  void parseNode(Node *node, Scene *scene);
  void parseSceneNode(Node *node, Scene *scene);
  void parseCameraNode(Node *node, Scene *scene);
  void parseLightNode(Node *node, Scene *scene);
  void parseShapeNode(Node *node, Scene *scene);
  void parseWavefrontNode(Node *node, Scene *scene);
  std::shared_ptr<Material> parseMaterialNode(Node *node, RGBAColor *color, ObjectType *type);
  std::shared_ptr<PNG> parseTextureNode(Node *node);
  void print(Node *node);
  void build(std::ifstream &filestream, Node *node);

  Node *root;
};

std::unique_ptr<Scene> parseFile(std::ifstream &filestream);
