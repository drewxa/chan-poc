#include <iostream>
#include <string>

#include "channel.h"
#include "selector.h"

using namespace channel;
using namespace std::chrono_literals;

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

template <typename Chan>
void Producer(Chan chan) {
  for (int i = 0; i < 5; ++i) {
    std::this_thread::sleep_for(100ms);
    chan << i;
  }
}
}  // namespace

int main() {
  using BuffChan = Chan<int[3]>;
  using SyncChan = Chan<std::thread::id>;

  BuffChan buff_chan;
  std::thread(Producer<BuffChan>, buff_chan).detach();

  SyncChan sync_chan;
  std::thread([sync_chan]() mutable {
    std::this_thread::sleep_for(300ms);
    sync_chan << std::this_thread::get_id();
  })
      .detach();

  auto tick = time::After(150ms);
  bool exit = false;
  auto stop = time::After(1000ms);

  for (auto selector = Selector::Create(); !exit;) {
    selector->Select(
        Case(buff_chan,
             [buff_chan]() mutable {
               std::cout << "buffer_chan recv: " << *buff_chan << std::endl;
             }),
        Case(sync_chan, []() { std::cout << "sync_chan recv" << std::endl; }),
        Case(tick, []() { std::cout << "tick recv" << std::endl; }),
        Case(stop, [&exit]() {
          std::cout << "stop recv" << std::endl;
          exit = true;
        }));
  }

  return 0;
}