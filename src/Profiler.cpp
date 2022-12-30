#include "macros.h"
#include "Profiler.h"

static Stats stats;

Profiler::~Profiler() {
  std::chrono::system_clock::time_point end = std::chrono::system_clock::now();
  std::chrono::duration<float> elapsed_seconds = end - start;
  stats.updateRuntime(f_, elapsed_seconds.count());
}

void Stats::print() {
  for (const auto & runtime : runtimes) {
    int minutes = runtime.second / 60;
    double seconds = runtime.second - minutes * 60;
    std::cout << runtime.first << ": " << minutes << " min " << std::fixed << std::setprecision(2) << seconds << " sec" << std::endl;
  }
}

void printStats() {
  stats.print();
}
