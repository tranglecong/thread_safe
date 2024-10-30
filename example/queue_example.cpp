#include "trlc/threadsafe/queue.hpp"

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <ostream>
#include <thread>

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