#include <iostream>
#include <iomanip>

#include "SafeProgressBar.h"

void displayRenderProgress(float progress, int barWidth) {
  // https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
  int pos = barWidth * progress;
  std::cout << "[";

  for (int i = 0; i < barWidth; ++i) {
    if (i < pos) std::cout << "=";
    else if (i == pos) std::cout << ">";
    else std::cout << " ";
  }

  std::cout << "] " << std::fixed << std::setprecision(2) << progress * 100.0 << " % \r";
  std::cout.flush();
}