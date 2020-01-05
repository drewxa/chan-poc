#include <iostream>
#include <string>

#include "channel.h"
#include "selector.h"

using namespace channel;
using namespace std::chrono_literals;

template <typename Chan>
void GetInt(Chan chan) {
  std::this_thread::sleep_for(500ms);
  chan << 42;
}

int main() {
  Chan<int> ch;
  Chan<std::string> str_chan;

  std::thread(GetInt<Chan<int>>, ch).detach();

  std::thread([str_chan]() mutable {
    std::this_thread::sleep_for(1000ms);
    str_chan << "Hello world!";
  })
      .detach();

  int i = 0;
  ch >> i;
  std::cout << i << std::endl;

  std::cout << *str_chan << std::endl;

  return 0;
}