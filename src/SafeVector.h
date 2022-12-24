#pragma once

#include <vector>
#include <mutex>

template <typename T>
class SafeVector {
public:
  template <class... Args>
  void emplace_back(Args&&... args) {
    std::lock_guard<std::mutex> lock(m);
    vec.emplace_back(std::forward<Args>(args)...);
  }

  T &at(size_t i) {
    std::lock_guard<std::mutex> lock(m);
    return vec.at(i);
  }

  size_t size() {
    std::lock_guard<std::mutex> lock(m);
    return vec.size();
  }

private:
  std::vector<T> vec;
  mutable std::mutex m;
};