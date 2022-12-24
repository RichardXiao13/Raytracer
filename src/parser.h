#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <memory>

#include "raytracer.h"

template <typename Out>
void split(const std::string &s, char delim, Out result);

std::vector<std::string> split(const std::string &s, char delim);

std::unique_ptr<Scene> readDataFromStream(std::istream& in);