#include "thread_safe/thread.hpp"

#include <chrono>
#include <iostream>
#include <string>
#include <thread>

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

    ThreadOnce thread_once{"thread once"};
    thread_once.setStartCallback([]() -> void
                                 { std::cout << "Thread started..." << std::endl; });
    thread_once.setResultCallback([](const int& result) -> void
                                  { std::cout << "Task result: " << result << std::endl; });
    thread_once.setExitCallback([]() -> void
                                { std::cout << "Thread finished." << std::endl; });
    // Invoke task for thread
    thread_once.invoke(addTask, 7, 8);
    thread_once.start(ThreadOnce::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();

    // Invoke task for thread
    thread_once.invoke(addTask, 99, 1);
    thread_once.start(ThreadOnce::RunMode::ONCE);
    // Wait for the thread to finish
    thread_once.stop();

    std::cout << "\n";

    // Example 2: Run a task in a loop until stopped
    std::cout << "Example 2: Run the task in a loop" << std::endl;

    LoopingThread looping_thread{"looping thread"};
    looping_thread.setStartCallback([]() -> void
                                    { std::cout << "Looping thread started..." << std::endl; });
    looping_thread.setResultCallback([](const std::string& result) -> void
                                     { std::cout << "Loop result: " << result << std::endl; });
    looping_thread.setExitCallback([]() -> void
                                   { std::cout << "Looping thread finished." << std::endl; });

    looping_thread.invoke(countTask, 10);
    looping_thread.start(LoopingThread::RunMode::LOOP);

    // Simulate some work in the main thread while the loop runs
    simulateWork(1050);
    looping_thread.stop();

    std::cout << "\n";

    // Example 3: Running a predicate-controlled loop
    std::cout << "Example 3: Loop with a predicate (runs 5 times)" << std::endl;

    int iteration_count = 0;
    PredThread pred_thread{"PredicateThread"};
    // Set result callback to print results and increment the count
    pred_thread.setResultCallback([&iteration_count](const std::string& result) -> void
                                  {
        std::cout << "Predicate loop result: " << result << std::endl;
        iteration_count++; });

    pred_thread.invoke(countTask, 10);
    // Set predicate
    pred_thread.setPredicate([&iteration_count]() -> bool
                             {
            return iteration_count < 5;  /*Stop after 5 iterations*/ });

    pred_thread.start(PredThread::RunMode::LOOP);
    // Simulate some work in the main thread while the loop runs
    simulateWork(1000);

    std::cout << "Predicate-controlled loop completed." << std::endl;

    return EXIT_SUCCESS;
}