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
