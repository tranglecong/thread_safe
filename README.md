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
    - [Queue Example](#queue-example)
    - [Thread Example](#thread-example)
    - [Variable Example](#variable-example)
    - [Wait Example](#wait-example)
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

## Usage

### Queue Example

### Thread Example

### Variable Example

### Wait Example

## Documentation

Full documentation can be found at <https://tranglecong.github.io/thread_safe/html>.

[CMake]: www.cmake.org
[git submodules]: http://git-scm.com/docs/git-submodule
