# Thread Safe

The **Thread Safe Library** is a C++ library that provides utilities for managing thread-safe operations. This library is designed to simplify concurrent programming by offering easy-to-use and robust thread-safe abstractions.

## Features

- **Queue**: A thread-safe queue with the ability to control pop and push operations, along with policies for discarding elements (oldest, newest, or no discard).
- **Variable**: A thread-safe variable manager, ensuring safe reads and writes across multiple threads.
- **Thread**: A thread manager that supports once mode and loop mode, can check results using callbacks and includes some other features.
- **Wait**: A mechanism to safely handle thread waiting and signaling.

## Installation

### Prerequisites

To use this library, you need:

- **CMake** 3.10 or higher
- **GCC**, **Clang** or **MSVC** compiler with C++17 support
- **GoogleTest** (automatically fetched by CMake for testing)

### Intergration

**_This library can be used as [CMake] subdirectory_**

1. Fetch it, e.g. using [git submodules] `git submodule add https://github.com/tranglecong/thread_safe` and `git submodule update --init --recursive`.

2. Call `add_subdirectory(ext/thread_safe)` or whatever your local path is to make it available in [CMake].

3. Simply call `target_link_libraries(your_target PUBLIC thread_safe)` to link this library and setups the include search path and compilation options.

**_You can also install thread safe library_**

1. Run CMake configure inside the library sources. If you do not want to build the UT set `-DTHREAD_SAFE_BUILD_TESTS=OFF`

    ```bash
    cmake -DCMAKE_BUILD_TYPE=Debug -DTHREAD_SAFE_BUILD_TESTS=ON -S . -B ./build
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
    find_package(thread_safe REQUIRED)
    target_link_libraries(your_target PUBLIC thread_safe)
    ```

## Example Code

### Queue

```c++
#include "thread_safe/queue.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <thread>

int main()
{
    using Queue = ThreadSafe::Queue<int>;
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

Refer to UT for more information on how to use it. [**_Thread Safe Queue Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_queue_test.cpp)

### Thread

```c++
#include "thread_safe/thread.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

int add_task(int a, int b)
{
    return a + b;
}

std::string count_task(int start)
{
    static int count{start};
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return std::string{"count = "} + std::string{std::to_string(count++)};
};

// Helper function to simulate work by sleeping for a few seconds
void simulateWork(int duration_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}

int main()
{
    using ThreadOnce = ThreadSafe::Thread<int, int, int>;
    using LoopingThread = ThreadSafe::Thread<std::string, int>;
    using PredThread = ThreadSafe::Thread<std::string, int>;

    // Example 1: Run the task once
    std::cout << "Example 1: Run the task once" << std::endl;

    // Create a thread to run the task
    ThreadOnce thread_once("thread once");
    // Set a callback to be called when the thread starts
    thread_once.setStartCallback([]() -> void
                                 { std::cout << "Thread started..." << std::endl; });
    // Set a callback to handle the result of the thread's task
    thread_once.setResultCallback([](const int& result) -> void
                                  { std::cout << "Task result: " << result << std::endl; });
    // Set a callback to be called when the thread exits
    thread_once.setExitCallback([]() -> void
                                { std::cout << "Thread finished." << std::endl; });
    // Invoke task for thread
    thread_once.invoke(add_task, 7, 8);
    // Start the thread
    thread_once.start(ThreadOnce::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();
    // Invoke task for thread
    thread_once.invoke(add_task, 99, 1);
    // Start the thread
    thread_once.start(ThreadOnce::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();

    std::cout << "\n";

    // Example 2: Run a task in a loop until stopped
    std::cout << "Example 2: Run the task in a loop" << std::endl;

    // Create a thread with a loop that performs the task repeatedly
    LoopingThread looping_thread("looping thread");
    // Set start callback
    looping_thread.setStartCallback([]() -> void
                                    { std::cout << "Looping thread started..." << std::endl; });
    // Set result callback to print the result on each iteration
    looping_thread.setResultCallback([](const std::string& result) -> void
                                     { std::cout << "Loop result: " << result << std::endl; });
    // Set an exit callback
    looping_thread.setExitCallback([]() -> void
                                   { std::cout << "Looping thread finished." << std::endl; });
    // Invoke task for thread
    looping_thread.invoke(count_task, 10);
    // Start the thread
    looping_thread.start(LoopingThread::RunMode::LOOP);

    // Simulate some work in the main thread while the loop runs
    simulateWork(1050);

    // Stop the loop
    looping_thread.stop();

    std::cout << "\n";

    // Example 3: Running a predicate-controlled loop
    std::cout << "Example 3: Loop with a predicate (runs 5 times)" << std::endl;

    int iterationCount = 0;

    // Create a thread that loops while a predicate is true
    PredThread pred_thread("PredicateThread");
    // Set result callback to print results and increment the count
    pred_thread.setResultCallback([&iterationCount](const std::string& result)
                                  {
        std::cout << "Predicate loop result: " << result << std::endl;
        iterationCount++; });

    // Invoke task for thread
    pred_thread.invoke(count_task, 10);
    // Set predicate
    pred_thread.setPredicate([&iterationCount]() -> bool
                             {
            return iterationCount < 5;  /*Stop after 5 iterations*/ });
    // Start the thread
    pred_thread.start(PredThread::RunMode::LOOP);
    // Simulate some work in the main thread while the loop runs
    simulateWork(1000);

    std::cout << "Predicate-controlled loop completed." << std::endl;

    return 0;
}

```

Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_thread_test.cpp)

### Variable

Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_variable_test.cpp)

### Wait

```c++
#include "thread_safe/wait.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    using Wait = ThreadSafe::Wait;
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

Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_wait_test.cpp)

## Documentation

Full documentation can be found at <https://tranglecong.github.io/thread_safe/html>.

[CMake]: www.cmake.org
[git submodules]: http://git-scm.com/docs/git-submodule
