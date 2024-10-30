#include "trlc/threadsafe/variable.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <vector>

// Test for basic functionality of Variable
TEST(ThreadSafeVariableTests, SetAndGetValue)
{
    trlc::threadsafe::Variable<int> var;

    var = 42;
    EXPECT_EQ(var.get(), 42);
    var = 100;
    EXPECT_EQ(var.get(), 100);
}

// Test invoking a non-member function
TEST(ThreadSafeVariableTests, InvokeNonMemberFunction)
{
    trlc::threadsafe::Variable<int> var{5};

    // Define a simple function that adds a number
    auto add = [](int val, int addend)
    { return val + addend; };

    // Invoke add function via the Variable class
    EXPECT_EQ(var.invoke(add, 10), 15); // 5 + 10 = 15
}

// Test invoking a member function on a stored value
TEST(ThreadSafeVariableTests, InvokeMemberFunction)
{
    trlc::threadsafe::Variable<std::vector<std::string>> var{"apple", "banana", "cherry"};

    // Invoke the `operator[]` member function
    EXPECT_EQ(var.invoke(&std::vector<std::string>::operator[], std::size_t{1}), "banana");
}

// Test invoking a const member function on a stored value
TEST(ThreadSafeVariableTests, InvokeConstMemberFunction)
{
    const trlc::threadsafe::Variable<std::vector<std::string>> var{"apple", "banana", "cherry"};

    // Invoke the const `operator[]` member function
    EXPECT_EQ(var.invoke(&std::vector<std::string>::operator[], std::size_t{2}), "cherry");
}

// Test thread safety
TEST(ThreadSafeVariableTests, ThreadSafety)
{
    trlc::threadsafe::Variable<int> var{0};

    auto add = [](int& val, int addend)
    { val = val + addend; };

    // Create multiple threads that increment the value
    std::vector<std::thread> threads;
    for (int i = 0; i < 1000; ++i)
    {
        threads.push_back(std::thread([&]()
                                      {
            for (int j = 0; j < 100; ++j) {
                var.invoke(add, 1);
            } }));
    }

    // Join all threads
    for (auto& t : threads)
    {
        t.join();
    }

    // The final value should be 1000 * 100 = 100000
    EXPECT_EQ(var.get(), 100000);
}

// Test default constructor and assignment operator
TEST(ThreadSafeVariableTests, DefaultConstructorAndAssignment)
{
    trlc::threadsafe::Variable<std::string> var;

    var = std::string{"Hello"};
    EXPECT_EQ(var.get(), "Hello");

    var = std::string{"World"};
    EXPECT_EQ(var.get(), "World");
}

// Test invoking with lambda function and capture
TEST(ThreadSafeVariableTests, InvokeWithLambda)
{
    trlc::threadsafe::Variable<int> var(10);

    // Lambda function that multiplies the value
    auto multiply = [](int val, int factor)
    { return val * factor; };

    // Invoke lambda through Variable class
    EXPECT_EQ(var.invoke(multiply, 3), 30); // 10 * 3 = 30
}

// Test const correctness when invoking const member functions
TEST(ThreadSafeVariableTests, ConstCorrectness)
{
    const trlc::threadsafe::Variable<std::vector<int>> var{1, 2, 3, 4};

    // Invoke const member function `operator[]` via const object
    EXPECT_EQ(var.invoke(&std::vector<int>::operator[], std::size_t{2}), 3);
}

// Test for empty constructor
TEST(ThreadSafeVariableTests, EmptyConstructor)
{
    trlc::threadsafe::Variable<std::string> var;
    EXPECT_EQ(var.get(), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}