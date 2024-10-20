#include "thread_safe/wait.hpp"

#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace ThreadSafe;

/**
 * @brief Test for notify function
 */
TEST(WaitTest, notifyTest)
{
    Wait w;
    std::thread notifier([&]()
                         {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        w.notify(); });

    EXPECT_EQ(w.wait(), Wait::Status::SUCCESS);
    notifier.join();
}

/**
 * @brief Test for waiting with a predicate (wait with a predicate)
 */
TEST(WaitTest, WaitWithPredicateTest)
{
    Wait w;
    std::atomic<bool> predCalled{false};

    std::thread predTrigger([&]()
                            {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        predCalled = true;
        w.notify(); });

    auto status = w.wait([&]() -> bool
                         { return predCalled.load(); });
    EXPECT_EQ(status, Wait::Status::SUCCESS);
    predTrigger.join();
}

/**
 * @brief Test for waitFor with timeout
 */
TEST(WaitTest, WaitForTimeoutTest)
{
    Wait w;
    auto status = w.waitFor(std::chrono::milliseconds(100));
    EXPECT_EQ(status, Wait::Status::TIMEOUT);
}

/**
 * @brief Test for waitFor with predicate and timeout
 */
TEST(WaitTest, WaitForWithPredicateAndTimeoutTest)
{
    Wait w;
    std::atomic<bool> predCalled{false};

    std::thread predTrigger([&]()
                            {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        predCalled = true;
        w.notify(); });

    auto status = w.waitFor(std::chrono::milliseconds(100), [&]() -> bool
                            { return predCalled.load(); });
    EXPECT_EQ(status, Wait::Status::SUCCESS);
    predTrigger.join();
}

/**
 * @brief Test for waitFor timeout when the predicate is not met
 */
TEST(WaitTest, WaitForPredicateTimeoutTest)
{
    Wait w;
    auto status = w.waitFor(std::chrono::milliseconds(100), []() -> bool
                            { return false; });
    EXPECT_EQ(status, Wait::Status::TIMEOUT);
}

/**
 * @brief Test for exit handling within wait
 */
TEST(WaitTest, WaitExitTest)
{
    Wait w;
    std::thread exitTrigger([&]()
                            {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        w.notify(); });

    auto status = w.wait();
    EXPECT_EQ(status, Wait::Status::SUCCESS);
    exitTrigger.join();
}

/**
 * @brief Test for handling exit request in predicate-based wait
 */
TEST(WaitTest, WaitWithExitTest)
{
    Wait w;
    std::atomic<bool> predCalled{false};

    std::thread predTrigger([&]()
                            {
        auto status = w.wait([&]() -> bool { return predCalled.load(); });
        EXPECT_EQ(status, Wait::Status::EXIT); });

    w.exit();
    predTrigger.join();
}

class MultithreadWaitTest : public ::testing::Test
{
protected:
    Wait w;
    std::atomic<bool> predCalled{false};
    std::atomic<bool> exitCalled{false};

    void SetUp() override
    {
        // Prepare any necessary setup before each test
        predCalled = false;
        exitCalled = false;
    }

    void runNotifyTest(std::function<void()> notifyFunc)
    {
        // Start thread that will notify the waiting threads
        std::thread notifier([&]()
                             {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            notifyFunc(); });

        // Start multiple threads that will call wait functions
        std::thread t1([&]()
                       { EXPECT_EQ(w.wait(), Wait::Status::SUCCESS); });
        std::thread t2(
            [&]()
            { EXPECT_EQ(w.wait([this]() -> bool
                               { return predCalled.load(); }),
                        Wait::Status::SUCCESS); });
        std::thread t3([&]()
                       { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(200)), Wait::Status::SUCCESS); });
        std::thread t4([&]()
                       { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(200), [this]() -> bool
                                             { return predCalled.load(); }),
                                   Wait::Status::SUCCESS); });

        // Join threads to wait for completion
        notifier.join();
        t1.join();
        t2.join();
        t3.join();
        t4.join();
    }
};

TEST_F(MultithreadWaitTest, NotifyAllTest)
{
    runNotifyTest([&]()
                  {
        // Set the predicate and notify all threads
        predCalled = true;
        w.notify(); });
}

TEST_F(MultithreadWaitTest, TimeoutTest)
{
    // Test with timeout
    std::thread t1([&]()
                   { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(100)), Wait::Status::TIMEOUT); });
    std::thread t2([&]()
                   { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(100), [this]() -> bool
                                         { return predCalled.load(); }),
                               Wait::Status::TIMEOUT); });

    // Join threads
    t1.join();
    t2.join();
}

TEST_F(MultithreadWaitTest, ExitTest)
{
    // Test with exit condition
    std::thread exitThread([&]()
                           {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        w.exit(); });

    std::thread t1([&]()
                   { EXPECT_EQ(w.wait(), Wait::Status::EXIT); });
    std::thread t2([&]()
                   { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(200)), Wait::Status::EXIT); });
    std::thread t3([&]()
                   { EXPECT_EQ(w.waitFor(std::chrono::milliseconds(200), [this]() -> bool
                                         { return predCalled.load(); }),
                               Wait::Status::EXIT); });

    // Join threads
    exitThread.join();
    t1.join();
    t2.join();
    t3.join();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}