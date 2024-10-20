#include "thread_safe/queue.hpp"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using Queue = ThreadSafe::Queue<int>;

// Utility function to simulate delay (sleep)
void sleep_ms(int milliseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

/**
 * @brief Test for basic push and pop functionality.
 */
TEST(QueueTest, BasicPushPop)
{
    Queue::Settings settings;
    Queue queue(settings);

    int popped_value;
    ASSERT_FALSE(queue.pop(popped_value, 100)); // Queue is empty, pop should fail.

    ASSERT_TRUE(queue.push(42));          // Push should succeed.
    ASSERT_TRUE(queue.pop(popped_value)); // Pop should succeed.
    ASSERT_EQ(popped_value, 42);          // Check popped value.
}

/**
 * @brief Test for queue size limitation and discard policy (DISCARD_OLDEST).
 */
TEST(QueueTest, DiscardOldest)
{
    Queue::Settings settings;
    settings.size = 2;
    settings.discard = Queue::Discard::DISCARD_OLDEST;

    Queue queue(settings);
    int discarded = -1;

    // Set up a discard callback.
    queue.setDiscardedCallback([&discarded](const int& elem)
                               { discarded = elem; });

    ASSERT_TRUE(queue.push(1));
    ASSERT_TRUE(queue.push(2));
    ASSERT_TRUE(queue.push(3)); // This will discard the oldest (1).

    int popped_value;
    ASSERT_TRUE(queue.pop(popped_value));
    ASSERT_EQ(popped_value, 2); // The oldest element was discarded, so 2 is next.
    ASSERT_EQ(discarded, 1);    // Ensure the discarded element was 1.
}

/**
 * @brief Test for queue size limitation and discard policy (DISCARD_NEWEST).
 */
TEST(QueueTest, DiscardNewest)
{
    Queue::Settings settings;
    settings.size = 2;
    settings.discard = Queue::Discard::DISCARD_NEWEST;

    Queue queue(settings);
    int discarded = -1;

    // Set up a discard callback.
    queue.setDiscardedCallback([&discarded](const int& elem)
                               { discarded = elem; });

    ASSERT_TRUE(queue.push(1));
    ASSERT_TRUE(queue.push(2));
    ASSERT_FALSE(queue.push(3)); // This will discard the newest (3).

    int popped_value;
    ASSERT_TRUE(queue.pop(popped_value));
    ASSERT_EQ(popped_value, 1); // The newest element (3) was discarded.
    ASSERT_EQ(discarded, 3);    // Ensure the discarded element was 3.
}

/**
 * @brief Test for queue size limitation with no discard policy (NO_DISCARD).
 */
TEST(QueueTest, NoDiscardPolicy)
{
    Queue::Settings settings;
    settings.size = 2;
    settings.discard = Queue::Discard::NO_DISCARD;

    Queue queue(settings);

    ASSERT_TRUE(queue.push(1));
    ASSERT_TRUE(queue.push(2));
    ASSERT_FALSE(queue.push(3, 100)); // No discard, push should fail when full.

    int popped_value;
    ASSERT_TRUE(queue.pop(popped_value));
    ASSERT_EQ(popped_value, 1); // The oldest element is still there.
}

/**
 * @brief Test for controlling push and pop operations.
 */
TEST(QueueTest, PushPopControl)
{
    Queue::Settings settings;
    settings.control = Queue::Control::FULL_CONTROL;

    Queue queue(settings);
    queue.openPush(); // Explicitly open push.
    queue.openPop();  // Explicitly open pop.

    int popped_value;
    ASSERT_TRUE(queue.push(42));          // Push should succeed.
    ASSERT_TRUE(queue.pop(popped_value)); // Pop should succeed.
    ASSERT_EQ(popped_value, 42);          // Check popped value.

    queue.closePush();             // Close push.
    ASSERT_FALSE(queue.push(100)); // Push should now fail.

    queue.closePop();                      // Close pop.
    ASSERT_FALSE(queue.pop(popped_value)); // Pop should now fail.
}

/**
 * @brief Test for concurrency - multiple threads pushing and popping.
 */
TEST(QueueTest, ConcurrentPushPop)
{
    Queue::Settings settings;
    settings.size = 10;

    Queue queue(settings);

    // Thread to push elements
    std::thread producer([&]()
                         {
        for (int i = 0; i < 10; ++i) {
            ASSERT_TRUE(queue.push(i));
            sleep_ms(10);  // Simulate some delay
        } });

    // Thread to pop elements
    std::thread consumer([&]()
                         {
        int popped_value;
        for (int i = 0; i < 10; ++i) {
            ASSERT_TRUE(queue.pop(popped_value));
            std::cout << "Popped: " << popped_value << std::endl;
        } });

    producer.join();
    consumer.join();
}

/**
 * @brief Test for push timeout.
 */
TEST(QueueTest, PushTimeout)
{
    Queue::Settings settings;
    Queue queue(settings);

    int popped_value;
    ASSERT_FALSE(queue.pop(popped_value, 100)); // Pop should time out.
}

/**
 * @brief Test for push with WAIT_FOREVER timeout.
 */
TEST(QueueTest, PushWaitForever)
{
    Queue::Settings settings;
    Queue queue(settings);

    std::thread producer([&]()
                         {
        sleep_ms(200);  // Simulate delayed push
        ASSERT_TRUE(queue.push(42)); });

    int popped_value;
    ASSERT_TRUE(queue.pop(popped_value)); // Wait forever for push.
    ASSERT_EQ(popped_value, 42);

    producer.join();
}

/**
 * @brief Test for handling maximum size limit of queue.
 */
TEST(QueueTest, MaxSizeLimit)
{
    Queue::Settings settings;
    settings.size = 1;

    Queue queue(settings);

    ASSERT_TRUE(queue.push(42));       // First push succeeds.
    ASSERT_FALSE(queue.push(30, 100)); // Next push fails because the queue is full.
}

/**
 * @brief Test for setting and triggering the discard callback.
 */
TEST(QueueTest, DiscardCallback)
{
    Queue::Settings settings;
    settings.size = 1;
    settings.discard = Queue::Discard::DISCARD_OLDEST;

    Queue queue(settings);
    int discarded = -1;

    queue.setDiscardedCallback([&discarded](const int& elem)
                               { discarded = elem; });

    ASSERT_TRUE(queue.push(42));
    ASSERT_TRUE(queue.push(100)); // This will discard 42.
    ASSERT_EQ(discarded, 42);
}

/**
 * @brief Test for behavior when queue is closed for push and pop.
 */
TEST(QueueTest, ClosedQueue)
{
    Queue::Settings settings;
    settings.control = Queue::Control::FULL_CONTROL;
    Queue queue(settings);

    queue.closePush();
    queue.closePop();

    int value;
    ASSERT_FALSE(queue.push(42));   // Push should fail when queue is closed.
    ASSERT_FALSE(queue.pop(value)); // Pop should fail when queue is closed.
}

/**
 * @brief Test for waitPushOpen and waitPopOpen.
 */
TEST(QueueTest, WaitOpenTest)
{
    Queue::Settings settings;
    settings.control = Queue::Control::FULL_CONTROL;
    Queue queue(settings);

    queue.closePush();
    ASSERT_FALSE(queue.waitPushOpen(100)); // Wait for push should time out.

    queue.openPush();
    ASSERT_TRUE(queue.waitPushOpen(100)); // Now it should succeed.

    queue.closePop();
    ASSERT_FALSE(queue.waitPopOpen(100)); // Wait for pop should time out.

    queue.openPop();
    ASSERT_TRUE(queue.waitPopOpen(100)); // Now it should succeed.
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
