#include <algorithm>
#include "raytracer.h"

using std::max;

double getRayX(double x, int w, int h) {
  return (2 * x - w) / max(w, h);
}
double getRayY(double y, int w, int h) {
  return (h - 2 * y) / max(w, h);
}