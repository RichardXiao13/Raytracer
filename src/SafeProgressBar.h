#pragma once

#include <mutex>
#include <iostream>
#include <iomanip>

void displayRenderProgress(double progress, int barWidth=70);

class SafeProgressBar {
public:
  SafeProgressBar() : counter(0), barWidth(70), total(100), update(1.0) {};
  SafeProgressBar(int width, int total, int update) : counter(0), barWidth(width), total(total), update(update) {};
  void increment() {
    std::lock_guard<std::mutex> lock(m);
    ++counter;
    if (counter % update == 0) {
      displayRenderProgress(static_cast<double>(counter) / total, barWidth);
    }
  }
private:
  mutable std::mutex m;
  int counter;
  int barWidth;
  int total;
  int update;
};