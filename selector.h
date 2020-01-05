#include <functional>
#include <memory>
#include <mutex>
#include <queue>

#include "channel.h"

namespace channel {

namespace detail {
template <typename Event, typename Handler>
struct CaseImpl {
  Event event;
  Handler handler;
};

template <typename... Args>
void SelectImpl(std::shared_ptr<Selector>&& selector);

template <typename C, typename... Args>
void SelectImpl(std::shared_ptr<Selector>&& selector, C&& cs, Args&&... args);

}  // namespace detail

template <typename Event, typename Handler>
detail::CaseImpl<Event, Handler> Case(Event event, Handler handler) {
  return {event, handler};
}

class Selector : public std::enable_shared_from_this<Selector> {
  template <typename T, size_t N>
  friend class detail::BufferChannel;

  template <typename T>
  friend class detail::SyncChannel;

  template <typename... Args>
  friend void detail::SelectImpl(std::shared_ptr<Selector>&& selector);

  template <typename C, typename... Args>
  friend void detail::SelectImpl(std::shared_ptr<Selector>&& selector,
                                 C&& cs,
                                 Args&&... args);

  void Notify(std::function<void()> handler);

  void Run();

  template <typename T, typename Handler>
  void Add(Chan<T> chan, Handler handler) {
    chan.data_->Select(this->shared_from_this(), std::move(handler));
  }

 public:
  template <typename... Args>
  void Select(Args&&... args) {
    detail::SelectImpl(this->shared_from_this(), std::forward<Args&&>(args)...);
  }

  static std::shared_ptr<Selector> Create();

 private:
  std::queue<std::function<void()>> queue_;
  std::condition_variable cv_;
  std::mutex mutex_;
};

namespace detail {
template <typename... Args>
void SelectImpl(std::shared_ptr<Selector>&& selector) {
  selector->Run();
}

template <typename C, typename... Args>
void SelectImpl(std::shared_ptr<Selector>&& selector, C&& cs, Args&&... args) {
  selector->Add(cs.event, cs.handler);
  SelectImpl(std::move(selector), std::forward<Args&&>(args)...);
}

template <typename... Args>
void SelectImpl(std::shared_ptr<Selector>&& selector, Args&&... args) {
  SelectImpl(std::move(selector), std::forward<Args&&>(args)...);
}
}  // namespace detail

template <typename... Args>
void Select(Args&&... args) {
  std::shared_ptr<Selector> selector(new Selector());
  SelectImpl(selector, std::forward<Args&&>(args)...);
}

}  // namespace channel
