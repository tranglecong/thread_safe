# TRLC ThreadSafe

![GCC](https://github.com/tranglecong/trlc_threadsafe/actions/workflows/GCC.yml/badge.svg)
![Clang](https://github.com/tranglecong/trlc_threadsafe/actions/workflows/Clang.yml/badge.svg)
![MSVC](https://github.com/tranglecong/trlc_threadsafe/actions/workflows/MSVC.yml/badge.svg)

The **TRLC ThreadSafe** is a C++ library that provides utilities for managing thread-safe operations. This library is designed to simplify concurrent programming by offering easy-to-use and robust thread-safe abstractions.

## Features

- **Queue**: A thread-safe queue with the ability to control pop and push operations, along with policies for discarding elements (oldest, newest, or no discard).
- **Variable**: A thread-safe variable manager, ensuring safe reads and writes across multiple threads.
- **Thread**: A thread manager that supports once mode and loop mode, can check results using callbacks and includes some other features.
- **Wait**: A mechanism to safely handle thread waiting and signaling.

## Example Code

### Queue

```c++
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <thread>
#include <trlc/threadsafe/queue.hpp>

int main()
{
    using Queue = trlc::threadsafe::Queue<int>;
    // Queue settings: size = 3, discard oldest elements when full, full control over push/pop
    Queue::Settings settings;
    settings.size = 3;
    settings.discard = Queue::Discard::DISCARD_OLDEST;
    settings.control = Queue::Control::FULL_CONTROL;

    // Create the thread-safe queue with the given settings
    Queue queue(settings);
    // Set a callback to be called when an element is discarded
    queue.setDiscardedCallback(
        [](const int& discardedElem)
        { std::cout << "Discarded element: " << discardedElem << std::endl; });

    std::atomic<bool> running{true};
    std::thread consumerThread{
        [&]() -> void
        {
            std::cout << "[Consumer]: Waiting queue to open for pop." << std::endl;
            if (!queue.waitPopOpen())
            {
                std::cerr << "[Consumer]: Failed to wait pop open!" << std::endl;
                return;
            }
            std::cout << "[Consumer]: The queue has opened for pop." << std::endl;
            while (running)
            {
                int elem{};
                if (!queue.pop(elem))
                {
                    continue;
                }
                std::cout << "[Consumer]: " << elem << std::endl;
            };
        }};
    // Push elements into the queue
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    std::cout << "Pushing elements into the queue..." << std::endl;
    queue.openPush();
    queue.push(1);
    queue.push(2);
    queue.push(3); // Queue is now full
    // Try to push another element, this should discard the oldest (1)
    queue.push(4); // Oldest element (1) will be discarded
    queue.openPop();
    queue.closePush();
    queue.push(5); // Will not push due to push closed
    std::this_thread::sleep_for(std::chrono::milliseconds{100});

    // Push more elements
    queue.openPush();
    queue.push(6);
    queue.push(7);
    queue.push(8);
    queue.openPop();
    std::this_thread::sleep_for(std::chrono::milliseconds{100});
    running = false;
    queue.closePush();
    queue.closePop();
    consumerThread.join();

    return EXIT_SUCCESS;
}
```

### Thread

```c++
#include <chrono>
#include <iostream>
#include <string>
#include <thread>
#include <trlc/threadsafe/thread.hpp>

void print(const std::string msg)
{
    std::cout << msg << std::endl;
}

int addTask(int a, int b)
{
    return a + b;
}

std::string countTask(int start)
{
    static int count{start};
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::string{"count = "} + std::string{std::to_string(count++)};
};

// Helper function to simulate work by sleeping
void simulateWork(int duration_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}

int main()
{
    using Thread = trlc::threadsafe::Thread;

    // Example 1: Run the task once
    std::cout << "Example 1: Run the task once" << std::endl;

    Thread thread_once{"thread once"};
    thread_once.setStartCallback([]() -> void
                                 { std::cout << "Thread started..." << std::endl; });
    thread_once.setResultCallback([](const Thread::ResultType& result) -> void
                                  { std::cout << "Result callback called! " << std::endl; });
    thread_once.setExitCallback([]() -> void
                                { std::cout << "Thread finished." << std::endl; });
    // Invoke task for thread
    thread_once.invoke(print, "Hello World");
    thread_once.run(Thread::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();

    // Invoke task for thread
    thread_once.invoke(print, "From Thread<>");
    thread_once.run(Thread::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();

    std::cout << "\n";

    // Example 2: Run a task in a loop until stopped
    std::cout << "Example 2: Run the task in a loop" << std::endl;

    Thread looping_thread{"looping thread"};
    looping_thread.setStartCallback([]() -> void
                                    { std::cout << "Looping thread started..." << std::endl; });
    looping_thread.setResultCallback([](const Thread::ResultType& result) -> void
                                     { std::cout << "Loop result: " << std::any_cast<std::string>(result) << std::endl; });
    looping_thread.setExitCallback([]() -> void
                                   { std::cout << "Looping thread finished." << std::endl; });

    looping_thread.invoke(countTask, 10);
    looping_thread.run(Thread::RunMode::LOOP);

    // Simulate some work in the main thread while the loop runs
    simulateWork(1050);
    looping_thread.stop();

    std::cout << "\n";

    // Example 3: Running a predicate-controlled loop
    std::cout << "Example 3: Loop with a predicate (runs 5 times)" << std::endl;

    int iteration_count = 0;
    Thread pred_thread{"PredicateThread"};
    // Set result callback to print results and increment the count
    pred_thread.setResultCallback([&iteration_count](const Thread::ResultType& result) -> void
                                  {
        std::cout << "Predicate loop result: " << std::any_cast<std::string>(result) << std::endl;
        iteration_count++; });

    pred_thread.invoke(countTask, 10);
    // Set predicate
    pred_thread.setPredicate([&iteration_count]() -> bool
                             {
            return iteration_count < 5;  /*Stop after 5 iterations*/ });

    pred_thread.run(Thread::RunMode::LOOP);
    // Simulate some work in the main thread while the loop runs
    simulateWork(1000);

    std::cout << "Predicate-controlled loop completed." << std::endl;

    return EXIT_SUCCESS;
}
```

### Variable

```c++
#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>
#include <trlc/threadsafe/variable.hpp>

// Helper function to simulate work by sleeping
void simulateWork(int duration_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}

int main()
{
    using Variable = trlc::threadsafe::Variable<std::string>;

    Variable var{"Initial"};
    std::atomic<bool> running{true};

    // Reading and print out value
    std::thread readingThread{[&]() -> void
                              {
                                  static std::string previous_value{};
                                  while (running)
                                  {
                                      if (var != previous_value)
                                      {
                                          previous_value = var;
                                          std::cout << "Current value: " << var.get() << std::endl;
                                      }
                                      std::this_thread::sleep_for(std::chrono::milliseconds(1));
                                  };
                              }};
    // Change value by assignment operator
    simulateWork(10);
    var = "0";
    simulateWork(10);
    // Use invoke to call member function
    var.invoke([](Variable::Type& s)
               { s.append("1"); });

    std::cout << "Current size: " << var.invoke([](const Variable::Type& s)
                                                { return s.size(); })
              << std::endl;

    simulateWork(10);
    // Use comparison operator
    var = "Example";
    std::cout << (var == "Example") << std::endl;
    std::cout << (var != "Example") << std::endl;
    std::cout << (var >= "Example") << std::endl;
    std::cout << (var <= "Example") << std::endl;
    std::cout << (var > "Example") << std::endl;
    std::cout << (var < "Example") << std::endl;

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    running = false;
    readingThread.join();

    return EXIT_SUCCESS;
}
```

### Wait

```c++
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <trlc/threadsafe/wait.hpp>

int main()
{
    using Wait = trlc::threadsafe::Wait;
    Wait wait;
    // Thread 1: Will Timeout
    std::thread t1([&]()
                   {
        std::cout << "[Thread 1]: Waiting for signal..." << std::endl;
        if (Wait::Status::TIMEOUT == wait.waitFor(std::chrono::milliseconds{100}))
        {
            std::cout << "[Thread 1]: Timeout!" << std::endl;
        }
        std::cout << "[Thread 1]: Leave!" << std::endl; });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Thread 2: Wait Predicate
    std::atomic<bool> pred_flag{false};
    auto pred = [&pred_flag]() -> bool
    { return pred_flag; };
    std::thread t2([&]()
                   {
    std::cout << "[Thread 2]: Waiting for predicate..." << std::endl;
    if (Wait::Status::SUCCESS == wait.wait(pred))
    {
        std::cout << "[Thread 2]: Predicate!" << std::endl;
    } 
    std::cout << "[Thread 2]: Leave!" << std::endl; });
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    // Thread 3: Will Timeout
    std::thread t3([&]()
                   {
    std::cout << "[Thread 3]: Waiting for predicate..." << std::endl;
    if (Wait::Status::TIMEOUT == wait.waitFor(std::chrono::milliseconds{200}, []()->bool{return false;}))
    {
        std::cout << "[Thread 3]: Timeout!" << std::endl;
    } 
    std::cout << "[Thread 3]: Leave!" << std::endl; });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    pred_flag = true;
    wait.notify();
    t1.join();
    t2.join();
    t3.join();

    return EXIT_SUCCESS;
}
```

## Installation

### Prerequisites

To use this library, you need:

- **CMake** 3.15 or higher
- **GCC**, **Clang** or **MSVC** compiler with C++17 support
- **GoogleTest** (automatically fetched by CMake for testing)

### Integration

#### Subdirectory

This library can be used as CMake subdirectory.

1. Fetch it, e.g. using [git submodules]:

```console
git submodule add https://github.com/tranglecong/trlc_threadsafe
git submodule update --init --recursive
```

Or you can use git clone: `git clone https://github.com/tranglecong/trlc_threadsafe.git`

1. Call `add_subdirectory(path_to/trlc_threadsafe)` or whatever your local path is to make it available in CMake file.

2. Simply call `target_link_libraries(your_target PUBLIC trlc::threadsafe)` to link this library and setups the include search path and compilation options.

#### Install Library

You can also install trlc_threadsafe library

1. Run CMake configure inside the library sources. If you want to build the UT and example set `-DTRLC_BUILD_TESTS=ON` , `-DTRLC_BUILD_EXAMPLES=ON`

    ```bash
    cmake -DCMAKE_BUILD_TYPE=Debug -DTRLC_BUILD_TESTS=OFF -DTRLC_BUILD_EXAMPLES=OFF -S . -B ./build
    ```

2. Build and install the library under `${CMAKE_INSTALL_PREFIX}`. You may be required to have sudo privileges to install in the `/usr/*`.

    ```bash
    cmake --build ./build -j8 -- install
    ```

    [Optional] if you want to run UT.

    ```bash
    ctest --test-dir ./build
    ```

3. To use an installed library.

    ```cmake
    find_package(trlc REQUIRED)
    target_link_libraries(your_target PUBLIC trlc::threadsafe)
    ```

## Documentation

Full documentation can be found at <https://tranglecong.github.io/trlc_threadsafe>.

## Contributing

Welcome contributions from everyone! If you’d like to help improve this project.
Thank you for considering contributing to this project!
