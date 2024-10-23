#include "thread_safe/variable.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <string>
#include <thread>

// Helper function to simulate work by sleeping
void simulateWork(int duration_ms)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(duration_ms));
}

int main()
{
    using Variable = ThreadSafe::Variable<std::string>;

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