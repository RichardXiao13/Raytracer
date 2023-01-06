#pragma once

#include "macros.h"

void displayRenderProgress(float progress, int barWidth=70);

class SafeProgressBar {
public:
  SafeProgressBar() : counter(0), barWidth(70), total(100), update(1.0), inc(1.0) {
    displayRenderProgress(0.0f, barWidth);
  };
  SafeProgressBar(int width, int total, int update) : counter(0), barWidth(width), total(total), update(update), inc(update) {
    displayRenderProgress(0.0f, barWidth);
  };
  void increment(int amount=1) {
    std::lock_guard<std::mutex> lock(m);
    counter += amount;
    if (counter == total || counter >= update) {
      displayRenderProgress(static_cast<float>(counter) / total, barWidth);
      update += inc;
    }
    if (counter == total) {
      std::cout << std::endl;
    }
  }
private:
  mutable std::mutex m;
  int counter;
  int barWidth;
  int total;
  int update;
  int inc;
};
