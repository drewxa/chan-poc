#include <iostream>
#include <string>

#include "channel.h"
#include "selector.h"

using namespace channel;
using namespace std::chrono_literals;

template <typename Chan>
void GetInt(Chan chan) {
  for (int i = 0; i < 20; ++i) {
    std::this_thread::sleep_for(10ms);
    chan << std::this_thread::get_id();
  }
}

int main() {
  using BuffChan = Chan<std::thread::id[20]>;
  BuffChan chan;
  
  std::thread(GetInt<BuffChan>, chan).detach();
  std::thread(GetInt<BuffChan>, chan).detach();
  std::thread(GetInt<BuffChan>, chan).detach();

  for (int i = 0; i < 60; ++i) {
    std::thread::id out{};
    chan >> out;
    std::cout << out << ": " << i << std::endl;
  }

  return 0;
}