#include <ctime>
#include <iostream>
#include <string>

#include "channel.h"
#include "selector.h"

using namespace channel;

namespace {
namespace time {

template <typename Duration>
auto After(Duration dur) {
  Chan<std::chrono::high_resolution_clock::time_point> result;
  std::thread th([result, dur]() mutable {
    std::this_thread::sleep_for(dur);
    result << std::chrono::high_resolution_clock::now();
  });
  th.detach();
  return result;
}

}  // namespace time
}  // namespace

int main() {
  using namespace std::chrono;
  using namespace std::chrono_literals;

  auto begin_time = std::chrono::high_resolution_clock::now();
  
  auto tick = time::After(1400ms);
  std::this_thread::sleep_for(1200ms);

  high_resolution_clock::time_point end_time;
  tick >> end_time;

  auto ms = duration_cast<milliseconds>(end_time - begin_time);
  std::cout << "Tick took " << ms.count() << "ms." << std::endl;

  return 0;
}
