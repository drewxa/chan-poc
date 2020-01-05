#pragma once

#include <array>
#include <memory>
#include <mutex>

namespace channel {
class Selector;

template <typename T>
class Chan;

namespace detail {

template <typename T>
class SyncChannel {
  template <typename T>
  friend class Chan;

  friend class channel::Selector;

  using value_type = T;

  private:

  SyncChannel() {}

  void Send(value_type value) {
    std::unique_lock<std::mutex> lk(mutex_);

    while (has_value_) {
      cv_.wait(lk);
    }

    value_ = std::move(value);
    has_value_ = true;
    lk.unlock();

    cv_.notify_all();
    if (selector_)
      selector_->Notify(handler_);
  }

  void Recv(value_type& value) {
    std::unique_lock<std::mutex> lk(mutex_);

    while (!has_value_) {
      cv_.wait(lk);
    }
    value = std::move(value_);
    has_value_ = false;
    lk.unlock();
    cv_.notify_all();
  }

 private:

  template <typename Handler>
  void Select(std::shared_ptr<Selector> selector, Handler handler) {
    selector_ = std::move(selector);
    handler_ = [handler]() mutable { handler(); };
  }
  
  bool has_value_ = false;
  value_type value_;
  std::mutex mutex_;
  std::condition_variable cv_;

  std::shared_ptr<Selector> selector_;
  std::function<void()> handler_;
};

template <typename T, size_t N>
class BufferChannel {
  using value_type = T;

  template <typename T>
  friend class Chan;

  friend class Selector;

  struct CircleQueue {
    size_t read_index_ = 0;
    size_t write_index_ = 0;
    size_t size_ = 0;
    std::array<T, N> data_;

    void Push(value_type value) {
      data_[write_index_] = std::move(value);
      write_index_ = (write_index_ + 1) % N;
      ++size_;
    }

    void Pop(value_type& value) {
      value = std::move(data_[read_index_]);
      read_index_ = (read_index_ + 1) % N;
      --size_;
    }

    bool Empty() const { return size_ == 0; }
    bool IsFull() const { return size_ == N; }
  };

 private:
  BufferChannel() {}

  void Send(value_type value) {
    std::unique_lock<std::mutex> lk(mutex_);

    while (data_.IsFull()) {
      cv_.wait(lk);
    }

    data_.Push(std::move(value));
    lk.unlock();
    cv_.notify_all();
    if (selector_)
      selector_->Notify(handler_);
  }

  void Recv(value_type& value) {
    std::unique_lock<std::mutex> lk(mutex_);

    while (data_.Empty()) {
      cv_.wait(lk);
    }

    data_.Pop(value);
    lk.unlock();
    cv_.notify_all();
  }

 private:
  template <typename Handler>
  void Select(std::shared_ptr<Selector> selector, Handler handler) {
    selector_ = std::move(selector);
    handler_ = [handler]() mutable { handler(); };
  }

  CircleQueue data_;
  std::mutex mutex_;
  std::condition_variable cv_;
  std::shared_ptr<Selector> selector_;
  std::function<void()> handler_;
};
}  // namespace detail

template <typename T>
class Chan {
 public:
  Chan() : data_(new detail::SyncChannel<T>()) {}

  void operator<<(T value) { data_->Send(std::move(value)); }
  void operator>>(T& value) { data_->Recv(value); }

  T operator*() {
    T value{};
    data_->Recv(value);
    return value;
  }

  friend class Selector;

 private:
  std::shared_ptr<detail::SyncChannel<T>> data_;
};

template <typename T, size_t N>
struct Chan<T[N]> {
  Chan() : data_(new detail::BufferChannel<T, N>()) {}

  void operator<<(T value) { data_->Send(std::move(value)); }
  void operator>>(T& value) { data_->Recv(value); }

  T operator*() {
    T value{};
    data_->Recv(value);
    return value;
  }

  friend class Selector;

 private:
  std::shared_ptr<detail::BufferChannel<T, N>> data_;
};

}  // namespace channel
