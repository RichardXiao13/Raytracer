#pragma once

#include "macros.h"
#include "raytracer.h"

enum class Tag {
  Scene,
  Camera,
  Light,
  Shape,
  Material,
  Texture,
  Wavefront,
  Unknown
};

static const std::vector<std::string> TagNames = {
  "Scene",
  "Camera",
  "Light",
  "Shape",
  "Material",
  "Texture",
  "Wavefront"
};

template <typename Out>
void split(const std::string &s, char delim, Out result);

inline bool ends_with(std::string const & value, std::string const & ending);

std::vector<std::string> split(const std::string &s, char delim);

std::vector<int> parseOBJPoint(const std::string &s);

std::unique_ptr<Scene> readDataFromStream(std::istream& in);

bool loadOBJ(
  const Vector3D &center,
  float scale,
  const std::string &filename,
  Scene *scene,
  const RGBAColor &color,
  const std::shared_ptr<Material> material
);

std::unique_ptr<Scene> readFromFile(const std::string& filename);