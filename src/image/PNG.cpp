#include "lodepng.h"
#include "PNG.h"

#include "../macros.h"
#include "../vector/vector3d.h"

float linearToGamma(float channel) {
  if (channel < 0.0031308) 
    return 12.92 * channel;

  return 1.055 * std::pow(channel, 1/2.4) - 0.055;
}

float gammaToLinear(float channel) {
  if (channel < 0.04045)
    return channel / 12.92;
  return std::pow((channel + 0.055) / 1.055, 2.4);
}

float exponentialExposure(float channel, float exposure) {
  return 1.0 - exp(-1 * channel * exposure);
}

RGBAColor RGBAColor::toSRGB() const {
  return RGBAColor(linearToGamma(r), linearToGamma(g), linearToGamma(b), a);
}

RGBAColor RGBAColor::toLinear() const {
  return RGBAColor(gammaToLinear(r), gammaToLinear(g), gammaToLinear(b), a);
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

RGBAColor operator*(const RGBAColor& c1, const RGBAColor& c2) {
  return RGBAColor(c1.r * c2.r, c1.g * c2.g, c1.b * c2.b, std::max(c1.a, c2.a));
}

RGBAColor &RGBAColor::operator*=(const RGBAColor& c) {
  r *= c.r;
  g *= c.g;
  b *= c.b;
  a = std::max(a, c.a);
  return *this;
}

RGBAColor &RGBAColor::operator/=(float scalar) {
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
  a = std::max(other.a, a);
  return *this;
}

bool hasNaN(RGBAColor &c) {
  return std::isnan(c.r) || std::isnan(c.g) || std::isnan(c.b);
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

RGBAColor &PNG::getPixel(unsigned row, unsigned col) {
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

bool PNG::readFromFile(const std::string &filename) {
  std::vector<unsigned char> byteData;
  unsigned error = lodepng::decode(byteData, width_, height_, filename);

  if (error) {
    std::cerr << "PNG decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
    return false;
  }

  delete[] image_;
  image_ = new RGBAColor[width_ * height_];

  for (unsigned i = 0; i < byteData.size(); i += 4) {
    RGBAColor &pixel = image_[i/4];
    pixel.r = byteData[i + 0] / 255.0f;
    pixel.g = byteData[i + 1] / 255.0f;
    pixel.b = byteData[i + 2] / 255.0f;
    pixel.a = byteData[i + 3] / 255.0f;
    pixel = pixel.toLinear();
  }

  return true;
}

bool PNG::saveToFile(const std::string& filename) {
  unsigned char *byteData = new unsigned char[width_ * height_ * 4];
  for (unsigned i = 0; i < width_ * height_; i++) {
    RGBAColor gammaCorrected = image_[i].toSRGB();
    byteData[(i * 4) + 0] = static_cast<unsigned char>(round(gammaCorrected.r * 255));
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