#pragma once

#include <string>

#include "vector3d.h"

/**
 * RGBAColor class
 * 
 * Save each component as a float to maintain bits of information across illumination.
*/
class RGBAColor {
public:
  RGBAColor() : r(0), g(0), b(0), a(1.0) {};
  RGBAColor(float r1, float g1, float b1) : r(r1), g(g1), b(b1), a(1.0) {};
  RGBAColor(float r1, float g1, float b1, float a1) : r(r1), g(g1), b(b1), a(a1) {};
  RGBAColor toSRGB();

  friend RGBAColor operator*(float scalar, const RGBAColor& c);
  friend RGBAColor operator*(const RGBAColor& c, float scalar);
  friend RGBAColor operator/(float scalar, const RGBAColor& c);
  friend RGBAColor operator/(const RGBAColor& c, float scalar);
  RGBAColor operator+(const RGBAColor& other) const;
  RGBAColor &operator+=(const RGBAColor& other);
  RGBAColor &operator*=(float scalar);
  RGBAColor &operator/=(float scalar);
  RGBAColor &operator*=(const RGBAColor& c);
  friend RGBAColor operator*(const RGBAColor& c1, const RGBAColor& c2);

  float r;
  float g;
  float b;
  float a;
};

RGBAColor operator*(const Vector3D& v, const RGBAColor& c);
RGBAColor operator*(const RGBAColor& c, const Vector3D& v);
class PNG {
public:
  PNG(int w, int h) : width_(w), height_(h) {
    image_ = new RGBAColor[width_ * height_];
  }

  PNG(const std::string &filename) {
    readFromFile(filename);
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

  bool readFromFile(const std::string &filename);

  bool saveToFile(const std::string &filename);

private:
  unsigned width_;
  unsigned height_;
  RGBAColor *image_;
};

std::ostream& operator<<(std::ostream& out, const RGBAColor& color);
float linearToGamma(float channel);
float exponentialExposure(float channel, float exposure);
RGBAColor clipColor(const RGBAColor& c);