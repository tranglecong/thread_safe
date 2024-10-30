#include "trlc/threadsafe/wait.hpp"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

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