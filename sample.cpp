#include <iostream>
#include <string>

#include "channel.h"
#include "selector.h"

using namespace channel;

int main() {
  Chan<int> ch;
  Chan<std::string[2]> ch1;
  Chan<int[2]> ch2;

  std::thread th([=]() mutable {
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    ch << 100;
  });
  std::thread th1([=]() mutable {
    std::this_thread::sleep_for(std::chrono::milliseconds(3000));
    ch1 << "string";
  });
  std::thread th2([=]() mutable {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ch2 << 3;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    ch2 << 31;
    ch2 << 32;
  });

  bool flag = true;
  std::shared_ptr<Selector> selector(new Selector());
  for (auto selector = Selector::Create(); flag;) {
    selector->Select(
        Case(ch,
             [ch, &flag]() mutable {
               std::cout << "chan1:" << *ch << std::endl;
               flag = false;
             }),
        Case(ch1,
             [ch1]() mutable { std::cout << "chan2:" << *ch1 << std::endl; }),
        Case(ch2,
             [ch2]() mutable { std::cout << "chan3:" << *ch2 << std::endl; }));
  }

  th.detach();
  th1.detach();
  th2.detach();
}