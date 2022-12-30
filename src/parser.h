#pragma once

#include "macros.h"
#include "raytracer.h"

template <typename Out>
void split(const std::string &s, char delim, Out result);

inline bool ends_with(std::string const & value, std::string const & ending);

std::vector<std::string> split(const std::string &s, char delim);

std::unique_ptr<Scene> readDataFromStream(std::istream& in);

std::unique_ptr<Scene> readOBJ(std::istream& in);

std::unique_ptr<Scene> readFromFile(const std::string& filename);