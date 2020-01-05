#include "selector.h"

namespace channel {

void Selector::Notify(std::function<void()> handler) {
  {
    std::lock_guard<std::mutex> lk(mutex_);

    queue_.push(std::move(handler));
  }
  cv_.notify_all();
}

void Selector::Run() {
  std::function<void()> task;
  {
    std::unique_lock<std::mutex> lk(mutex_);

    while (queue_.empty()) {
      cv_.wait(lk);
    }

    task = queue_.front();
    queue_.pop();
  }
  task();
}

// static
std::shared_ptr<Selector> Selector::Create() {
  return std::shared_ptr<Selector>(new Selector());
}
}  // namespace channel
