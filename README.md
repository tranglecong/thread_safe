# Thread Safe

The **Thread Safe Library** is a C++ library that provides utilities for managing thread-safe operations. This library is designed to simplify concurrent programming by offering easy-to-use and robust thread-safe abstractions.

## Features

- **Queue**: A thread-safe queue with the ability to control pop and push operations, along with policies for discarding elements (oldest, newest, or no discard).
- **Variable**: A thread-safe variable manager, ensuring safe reads and writes across multiple threads.
- **Thread**: A thread manager that supports once mode and loop mode, can check results using callbacks and includes some other features.
- **Wait**: A mechanism to safely handle thread waiting and signaling.

## Table of Contents

- [Thread Safe](#thread-safe)
  - [Features](#features)
  - [Table of Contents](#table-of-contents)
  - [Installation](#installation)
    - [Prerequisites](#prerequisites)
    - [Intergration](#intergration)
  - [Usage](#usage)
    - [Queue](#queue)
    - [Thread](#thread)
    - [Variable](#variable)
    - [Wait](#wait)
  - [Documentation](#documentation)

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

## Example

### Queue
```c++
#include "thread_safe/queue.hpp"

#include <chrono>
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
    std::this_thread::sleep_for(std::chrono::milliseconds{200});
    running = false;
    queue.closePop();
    queue.closePush();
    consumerThread.join();
    return 0;
}
```
Refer to UT for more information on how to use it. [**_Thread Safe Queue Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_queue_test.cpp)
### Thread


Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_thread_test.cpp)
### Variable

Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_variable_test.cpp)
### Wait

Refer to UT for more information on how to use it. [**_Thread Safe Thread Tests_**](https://github.com/tranglecong/thread_safe/blob/main/tests/thread_safe_wait_test.cpp)
## Documentation

Full documentation can be found at <https://tranglecong.github.io/thread_safe/html>.

[CMake]: www.cmake.org
[git submodules]: http://git-scm.com/docs/git-submodule
