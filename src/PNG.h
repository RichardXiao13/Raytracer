#pragma once

#include <string>

using namespace std;

/**
 * RGBAColor class
 * 
 * Save each component as a double to maintain bits of information across illumination.
*/
class RGBAColor {
public:
  RGBAColor() : r(0), g(0), b(0), a(1.0) {};
  RGBAColor(double r1, double g1, double b1) : r(r1), g(g1), b(b1), a(1.0) {};
  RGBAColor(double r1, double g1, double b1, double a1) : r(r1), g(g1), b(b1), a(a1) {};
  RGBAColor toSRGB();

  double r;
  double g;
  double b;
  double a;
};

class PNG {
public:
  PNG(int w, int h) : width_(w), height_(h) {
    image_ = new RGBAColor[width_ * height_];
  }

  ~PNG() {
    delete[] image_;
  }

  RGBAColor &getPixel(int row, int col);

  int width() {
    return width_;
  }

  int height() {
    return height_;
  }

  bool saveToFile(const string& filename);

private:
  int width_;
  int height_;
  RGBAColor *image_;
};

std::ostream& operator<<(std::ostream& out, const RGBAColor& color);
double linearToGamma(double channel);
