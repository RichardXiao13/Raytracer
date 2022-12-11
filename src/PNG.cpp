#include <iostream>
#include <string>
#include <cmath>

#include "PNG.h"
#include "lodepng.h"

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