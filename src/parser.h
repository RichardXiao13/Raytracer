#pragma once

#include <vector>
#include <string>
#include <fstream>

#include "raytracer.h"

using namespace std;

template <typename Out>
void split(const string &s, char delim, Out result);

vector<string> split(const string &s, char delim);

Scene *readDataFromStream(istream& in);