#include "macros.h"
#include "PNG.h"
#include "lodepng.h"
#include "vector3d.h"

float linearToGamma(float channel) {
  if (channel < 0.0031308) {
    return 12.92 * channel;
  }
  return 1.055 * pow(channel, 1/2.4) - 0.055;
}

float exponentialExposure(float channel, float exposure) {
  return 1.0 - exp(-1 * channel * exposure);
}

RGBAColor RGBAColor::toSRGB() {
  return RGBAColor(linearToGamma(r), linearToGamma(g), linearToGamma(b), a);
}

RGBAColor operator*(float scalar, const RGBAColor& c) {
  return RGBAColor(scalar * c.r, scalar * c.g, scalar * c.b, c.a);
}
RGBAColor operator*(const RGBAColor& c, float scalar) {
  return RGBAColor(scalar * c.r, scalar * c.g, scalar * c.b, c.a);
}

RGBAColor operator/(float scalar, const RGBAColor& c) {
  float inv = 1.0f / scalar;
  return RGBAColor(inv * c.r, inv * c.g, inv * c.b, c.a);
}

RGBAColor operator/(const RGBAColor& c, float scalar) {
  float inv = 1.0f / scalar;
  return RGBAColor(inv * c.r, inv * c.g, inv * c.b, c.a);
}

RGBAColor &RGBAColor::operator*=(float scalar) {
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
  return RGBAColor(c.r * v.x, c.g * v.y, c.b * v.z, c.a);
}

RGBAColor operator*(const RGBAColor& c, const Vector3D& v) {
  return RGBAColor(c.r * v.x, c.g * v.y, c.b * v.z, c.a);
}

RGBAColor clipColor(const RGBAColor& c) {
  return RGBAColor(
    std::max(0.0f, std::min(c.r, 1.0f)),
    std::max(0.0f, std::min(c.g, 1.0f)),
    std::max(0.0f, std::min(c.b, 1.0f)),
    std::max(0.0f, std::min(c.a, 1.0f))
  );
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

bool PNG::saveToFile(const std::string& filename) {
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
    std::cerr << "PNG encoding error " << error << ": " << lodepng_error_text(error) << std::endl;
  }
  delete[] byteData;
  return (error == 0);
}