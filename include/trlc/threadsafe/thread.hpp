#pragma once

#include "trlc/threadsafe/common.hpp"

#include <any>
#include <atomic>
#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <thread>
#include <type_traits>

namespace trlc
{
namespace threadsafe
{

/**
 * @brief Enum to represent thread priorities.
 */
enum class ThreadPriority : uint32_t
{
    LOWEST = 0,
    BELOW_NORMAL = 1,
    NORMAL = 2,
    ABOVE_NORMAL = 3,
    HIGHEST = 4,
    TIME_CRITICAL = 5
};

using NativeThreadPrioritys = std::map<ThreadPriority, int32_t>;

/**
 * @brief Provides the default mapping of ThreadPriority to native priority values.
 * @return The map of ThreadPriority to int32_t native priority values.
 */
const NativeThreadPrioritys& defaultNativeThreadPrioritys();

/**
 * @brief Sets the native thread priority for a given thread.
 * @param priority The desired thread priority.
 * @param native_handle The native handle of the thread to set priority.
 */
void setNaitiveThreadPriority(ThreadPriority priority, const std::thread::native_handle_type native_handle);

/**
 * @brief A thread class that supports custom functions, thread priorities, and callbacks.
 */
class Thread
{
public:
    using ResultType = std::any;
    using Callback = std::function<void()>;
    using ResultCallback = std::function<void(const ResultType&)>;
    using Callable = std::function<ResultType()>;
    using Pred = std::function<bool()>;

    /**
     * @brief Enum to represent whether the thread should run once or in a loop.
     */
    enum class RunMode : uint8_t
    {
        ONCE = 0,
        LOOP = 1
    };

    /**
     * @brief Constructor for creating a thread with a function and its arguments.
     * @param name The name of the thread.
     * @param priority The priority of the thread.
     */
    Thread(const std::string& name, const ThreadPriority priority = ThreadPriority::NORMAL)
        : m_name{name}
        , m_priority{priority}
    {
    }

    /**
     * @brief Destructor that ensures the thread stops when the object is destroyed.
     */
    ~Thread()
    {
        stop();
    }

    // Make this class uncopyable
    UNCOPYABLE(Thread);

    /**
     * @brief Sets the function and its arguments before starting the thread.
     * This function stores the function and arguments to be invoked when the thread is run.
     * @tparam Func The types of the function.
     * @tparam Args The types of arguments for the function.
     * @param func The function to be executed.
     * @param args The arguments to be passed to the function.
     * @return `true` if the function and arguments were successfully set, `false` otherwise.
     */
    template<typename Func, typename... Args>
    bool invoke(Func func, Args&&... args) noexcept(std::is_nothrow_invocable_v<Func, Args...>)
    {
        if (m_thread_ptr != nullptr)
        {
            return false;
        }

        // Store the function (converted to std::function<ResultType()>)
        m_callable = [func, tuple_args = std::tuple<Args...>(std::forward<Args>(args)...)]() mutable -> ResultType
        {
            using result_t = decltype(std::apply(func, tuple_args));
            ResultType result{};
            if constexpr (std::is_void_v<result_t>)
            {
                std::apply(func, tuple_args);
            }
            else
            {
                result = std::apply(func, tuple_args);
            }
            return result;
        };
        return true;
    }

    // Setters for member variables
    /**
     * @brief Sets the predicate to control the loop condition of the thread.
     * @param pred The predicate function that returns a boolean indicating whether the loop should continue.
     */
    void setPredicate(Pred pred)
    {
        m_pred = pred;
    }

    /**
     * @brief Sets the start callback function to be executed when the thread starts.
     * @param start_callback The callback function.
     */
    void setStartCallback(Callback start_callback)
    {
        m_start_callback = start_callback;
    }

    /**
     * @brief Sets the result callback function to be executed with the result of the function.
     * @param result_callback The callback function to handle the result.
     */
    void setResultCallback(ResultCallback result_callback)
    {
        m_result_callback = result_callback;
    }

    /**
     * @brief Sets the exit callback function to be executed when the thread exits.
     * @param exit_callback The callback function.
     */
    void setExitCallback(Callback exit_callback)
    {
        m_exit_callback = exit_callback;
    }

    /**
     * @brief Starts the thread.
     * @param mode Whether the thread should run once or in a loop.
     * @return `true` if start sucessfull, `false` otherwise.
     */
    bool run(const RunMode mode)
    {
        if (m_thread_ptr != nullptr)
        {
            return false;
        }
        if (!m_callable)
        {
            return false;
        }
        m_loop.store(false, std::memory_order_release);
        if (mode == RunMode::LOOP)
        {
            m_loop.store(true, std::memory_order_release);
        }
        m_thread_ptr = std::make_unique<std::thread>([this]()
                                                     { loop(); });
        return true;
    }
    /**
     * @brief Stops the thread's loop.
     * @return `true` if stop sucessfull, `false` otherwise.
     */
    bool stop()
    {
        m_loop.store(false, std::memory_order_release);
        if (!m_thread_ptr)
        {
            return false;
        }
        if (m_thread_ptr->joinable())
        {
            m_thread_ptr->join();
        }
        m_thread_ptr.reset();
        return true;
    }

    /**
     * @brief Returns the name of the thread.
     * @return The name of the thread.
     */
    std::string name() const
    {
        return m_name;
    }

private:
    const std::string m_name;
    Callable m_callable{nullptr};
    std::atomic<bool> m_loop{true};
    ThreadPriority m_priority;
    Pred m_pred{};
    Callback m_start_callback{};
    ResultCallback m_result_callback{};
    Callback m_exit_callback{};
    std::unique_ptr<std::thread> m_thread_ptr{};

    /**
     * @brief The main loop function that runs the thread.
     */
    void loop()
    {
        setNaitiveThreadPriority(m_priority, m_thread_ptr->native_handle());
        startCallback();

        do
        {
            call();
        } while (isContinue());

        exitCallback();
    }

    /**
     * @brief Function to execute the function stored in the thread.
     */
    void call()
    {
        if (m_callable)
        {
            ResultType result{};
            result = std::move(m_callable());
            if (m_result_callback)
            {
                m_result_callback(result);
            };
        }
    }

    /**
     * @brief Function to execute the start callback.
     */
    void startCallback()
    {
        if (m_start_callback)
        {
            m_start_callback();
        };
    }

    /**
     * @brief Function to execute the exit callback.
     */
    void exitCallback()
    {
        if (m_exit_callback)
        {
            m_exit_callback();
        };
    }

    /**
     * @brief Function to determine whether the thread should continue running.
     * @return True if the thread should continue running, false otherwise.
     */
    bool isContinue()
    {
        if (!m_loop.load(std::memory_order_acquire))
        {
            return false;
        }
        if (m_pred && !m_pred())
        {
            return false;
        }
        return true;
    }
};
} // namespace threadsafe
} // namespace trlc