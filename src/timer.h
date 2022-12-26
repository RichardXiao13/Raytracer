// from stackoverflow - create a timer wrapper around a function
// https://stackoverflow.com/questions/24468397/a-timer-for-arbitrary-functions
#pragma once

#include <type_traits>
#include <chrono>
 
template <class Fn, class... Args>
auto timer(Fn fn, Args && ... args)
    -> typename std::enable_if< 
    	!std::is_same< decltype( fn( std::forward<Args>(args) ... )), void >::value,
    	std::pair<float, decltype(fn(args...))> >::type
{
  static_assert(!std::is_void<decltype(fn(args...))>::value,
                "Call timer_void if return type is void!");
  auto start = std::chrono::high_resolution_clock::now();
  auto ret = fn(args...);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed_seconds = end - start;
  return { elapsed_seconds.count(), ret };
}
 
// If fn returns void, only the time is returned
template <class Fn, class ... Args>
auto timer(Fn fn, Args && ... args) -> typename std::enable_if< 
    	std::is_same< decltype( fn( std::forward<Args>(args) ... )), void >::value,
    	float>::type
{
  static_assert(std::is_void<decltype(fn(args...))>::value,
                "Call timer for non void return type");
  auto start = std::chrono::high_resolution_clock::now();
  fn(args...);
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<float> elapsed_seconds = end - start;
  return elapsed_seconds.count();
}