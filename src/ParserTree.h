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
bool findOptions(const std::string &line, std::vector<std::string> *options);
bool stringTupleToVector3D(const std::string &string, Vector3D *vector);
bool findType(const std::string &line, std::string *type);

bool parseDistantLightOptions(const std::vector<std::string> &options, Light **light);
bool parsePointLightOptions(const std::vector<std::string> &options, Light **light);

bool parseSphereOptions(
  const std::vector<std::string> &options,
  Sphere **sphere,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
);
bool parseTriangleOptions(
  const std::vector<std::string> &options,
  Triangle **triangle,
  RGBAColor color,
  ObjectType objectType,
  std::shared_ptr<Material> material,
  std::shared_ptr<PNG> texture
);
bool parsePlaneOptions(
  const std::vector<std::string> &options,
  Plane **plane,
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