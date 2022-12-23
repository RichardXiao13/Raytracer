#pragma once

#include <vector>
#include <string>
#include <fstream>
#include <memory>

#include "raytracer.h"

using namespace std;

template <typename Out>
void split(const string &s, char delim, Out result);

vector<string> split(const string &s, char delim);

unique_ptr<Scene> readDataFromStream(istream& in);