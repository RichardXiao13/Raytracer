#pragma once

#include "macros.h"
#include "PNG.h"

// Colors from Wikipedia
const std::unordered_map<std::string, RGBAColor> MaterialColors = {
  { "copper", RGBAColor(0.4793201831f, 0.1714411007f, 0.03310476657f) },
  { "pale copper", RGBAColor(0.7011018919f, 0.2541520943f, 0.1356333297f) },
  { "copper red", RGBAColor(0.5972017884f, 0.152926152f, 0.08228270713f) },
  { "copper penny", RGBAColor(0.4178850708f, 0.1589608351f, 0.1412632911f) },
  { "gold", RGBAColor(1.0f, 0.6795424696330938f, 0.0f) },
  { "metallic gold", RGBAColor(0.6583748172794485f, 0.4286904966139066f, 0.0382043715953465f) }
};