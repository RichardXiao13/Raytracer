#include <iostream>
#include <string>
#include <cmath>

#include "PNG.h"
#include "lodepng.h"
#include "vector3d.h"

using namespace std;

double linearToGamma(double channel) {
  if (channel < 0.0031308) {
    return 12.92 * channel;
  }
  return 1.055 * pow(channel, 1/2.4) - 0.055;
}

double exponentialExposure(double channel, double exposure) {
  return 1.0 - exp(-1 * channel * exposure);
}

RGBAColor RGBAColor::toSRGB() {
  return RGBAColor(linearToGamma(r), linearToGamma(g), linearToGamma(b), a);
}

RGBAColor operator*(double scalar, const RGBAColor& c) {
  return RGBAColor(scalar * c.r, scalar * c.g, scalar * c.b, c.a);
}
RGBAColor operator*(const RGBAColor& c, double scalar) {
  return RGBAColor(scalar * c.r, scalar * c.g, scalar * c.b, c.a);
}

RGBAColor &RGBAColor::operator*=(double scalar) {
  r *= scalar;
  g *= scalar;
  b *= scalar;
  return *this;
}

RGBAColor RGBAColor::operator+(const RGBAColor& other) const {
  return RGBAColor(other.r + r, other.g + g, other.b + b, 1.0);
}

RGBAColor &RGBAColor::operator+=(const RGBAColor& other) {
  r += other.r;
  g += other.g;
  b += other.b;
  return *this;
}

RGBAColor operator*(const Vector3D& v, const RGBAColor& c) {
  return RGBAColor(c.r * v[0], c.g * v[1], c.b * v[2], c.a);
}

RGBAColor operator*(const RGBAColor& c, const Vector3D& v) {
  return RGBAColor(c.r * v[0], c.g * v[1], c.b * v[2], c.a);
}

RGBAColor clipColor(const RGBAColor& c) {
  return RGBAColor(min(c.r, 1.0), min(c.g, 1.0), min(c.b, 1.0), min(c.a, 1.0));
}

RGBAColor &PNG::getPixel(int row, int col) {
  assert(row >= 0 && row < height_);
  assert(col >= 0 && col < width_);

  return image_[row * width_ + col];
}

std::ostream& operator<<(std::ostream& out, const RGBAColor& color) {
  out << '('
      << static_cast<unsigned>(color.r) << ", "
      << static_cast<unsigned>(color.g) << ", "
      << static_cast<unsigned>(color.b) << ", "
      << color.a
      << ')';
  return out;
}

bool PNG::saveToFile(const string& filename) {
  unsigned char *byteData = new unsigned char[width_ * height_ * 4];
  for (int i = 0; i < width_ * height_; i++) {
    RGBAColor gammaCorrected = image_[i].toSRGB();
    byteData[(i * 4)]     = static_cast<unsigned char>(round(gammaCorrected.r * 255));
    byteData[(i * 4) + 1] = static_cast<unsigned char>(round(gammaCorrected.g * 255));
    byteData[(i * 4) + 2] = static_cast<unsigned char>(round(gammaCorrected.b * 255));
    byteData[(i * 4) + 3] = static_cast<unsigned char>(round(gammaCorrected.a * 255));
  }

  unsigned error = lodepng::encode(filename, byteData, width_, height_);
  if (error) {
    cerr << "PNG encoding error " << error << ": " << lodepng_error_text(error) << endl;
  }
  delete[] byteData;
  return (error == 0);
}