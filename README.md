# PoC of Chan

Golang-like channels in C++.

#### Disclaimer
This is an inefficient implementation of golang channels in C++. It's just a working PoC.

## Usage

### Unbuffered channels
`channel::Chan<T>` is a simple as [Golang unbuffered Channels](https://gobyexample.com/channels).

```cpp
Chan<std::string> messages;

std::thread([messages]() mutable { messages << "ping"; }).detach();

std::string mgs;
messages >> mgs;
```

Create a new channel by defined a variable `Chan<T>`. Channels are typed by the values they convey.

Send a value into a channel using the channel `operator<<` syntax. Here we send "ping" to the messages channel we made above, from a new ~~goroutine~~ thread.

The `operator>>` syntax receives a value from the channel. Here we’ll receive the "ping" message we sent above and print it out.


### Channel Buffering
`channel::Chan<T[N]>` is like [Golang Channel Buffering](https://gobyexample.com/channel-buffering).

Buffered channels accept a limited number of values without a corresponding receiver for those values.

```cpp
Chan<std::string[2]> messages;

messages << "buffered";
messages << "channel";

std::string msg;
messages >> msg;
std::cout << msg << std::endl;
std::cout << *messages << std::endl;
```

Here we make a channel of strings buffering up to 2 values.

Because this channel is buffered, we can send these values into the channel without a corresponding concurrent receive.

Later we can receive these two values as usual or using the `operator*` syntax;

### Select
`channel::Selector` is an analog [Golang Select](https://gobyexample.com/select).

Go’s select lets you wait on multiple channel operations. Combining goroutines and channels with select is a powerful feature of Go.
So we need it in our PoC.

`channel::Selector::Select` is a blocking operation. It uses `std::conditional_variable` to wait for data in the channels.

```cpp
Chan<std::string> c1;
Chan<std::string> c2;

std::thread([c1]() mutable {
  std::this_thread::sleep_for(1s);
  c1 << "one";
}).detach();

std::thread([c2]() mutable {
  std::this_thread::sleep_for(2s);
  c2 << "two";
}).detach();

auto selector = Selector::Create();
for (int i = 0; i < 2; ++i) {
  selector->Select(
    Case(c1,
        [c1]() mutable {std::cout << "received" << *c1 << std::endl; }),
    Case(c2,
        [c2]() mutable {std::cout << "received" << *c2 << std::endl; })
  );
}
```

For our example we’ll select across two channels.

Each channel will receive a value after some amount of time, to simulate e.g. blocking RPC operations executing in concurrent goroutines.

We’ll use select to await both of these values simultaneously, printing each one as it arrives.
