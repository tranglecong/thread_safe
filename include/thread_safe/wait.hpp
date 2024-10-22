#pragma once

#include "common.hpp"

#include <atomic>
#include <condition_variable>
#include <mutex>

namespace ThreadSafe
{

/**
 * @brief A thread-safe class for handling conditional waits.
 *
 * This class provides mechanisms for waiting on a condition variable with support
 * for notifications, timeouts, and predicate-based conditions.
 */
class Wait
{
public:
    /**
     * @brief Enumeration representing the possible status outcomes for waiting.
     */
    enum class Status : uint32_t
    {
        SUCCESS = 0,
        TIMEOUT = 1,
        EXIT = 2
    };

    /**
     * @brief Default constructor for the Wait class.
     */
    Wait() = default;

    /**
     * @brief Default destructor for the Wait class.
     */
    ~Wait();

    // Make this class uncopyable
    UNCOPYABLE(Wait);
    /**
     * @brief Notify all waiting threads.
     *
     * Wakes up all threads that are currently blocked waiting on the condition variable.
     */
    void notify();

    /**
     * @brief Exit request to unblock waiting threads.
     *
     * Signals the internal exit flag and notifies all threads, allowing them to exit gracefully.
     */
    void exit();

    /**
     * @brief Block the calling thread until the condition is met or exit is requested.
     *
     * @return The status of the wait operation, either `Status::SUCCESS` or `Status::EXIT`.
     */
    Status wait();

    /**
     * @brief Block the calling thread until the condition is met or the predicate returns true.
     *
     * This function blocks the calling thread until either the specified predicate returns true
     * or an exit request is made.
     *
     * @tparam Pr The predicate type.
     * @param pred A callable predicate that returns a boolean value indicating whether to stop
     * waiting.
     * @return The status of the wait operation, either `Status::SUCCESS` or `Status::EXIT`.
     */
    template<typename Pr>
    Status wait(Pr pred);

    /**
     * @brief Block the calling thread for a specified duration or until exit is requested.
     *
     * The thread will be blocked for the specified timeout period unless an exit request is made
     * during that time.
     *
     * @tparam Repr The type of the duration's representation.
     * @tparam Period The period of the duration.
     * @param timeout The duration to wait for.
     * @return The status of the wait operation, either `Status::SUCCESS`, `Status::TIMEOUT`, or
     * `Status::EXIT`.
     */
    template<class Repr, class Period>
    Wait::Status waitFor(const std::chrono::duration<Repr, Period>& timeout);

    /**
     * @brief Block the calling thread for a specified duration or until the predicate returns true.
     *
     * The thread will be blocked for the specified timeout period, but will return early if the
     * predicate becomes true or an exit request is made.
     *
     * @tparam Repr The type of the duration's representation.
     * @tparam Period The period of the duration.
     * @tparam Pr The predicate type.
     * @param timeout The duration to wait for.
     * @param pred A callable predicate that returns a boolean value indicating whether to stop
     * waiting.
     * @return The status of the wait operation, either `Status::SUCCESS`, `Status::TIMEOUT`, or
     * `Status::EXIT`.
     */
    template<class Repr, class Period, typename Pr>
    Status waitFor(const std::chrono::duration<Repr, Period>& timeout, Pr pred);

private:
    mutable std::mutex m_lock;                     ///< Mutex for thread-safe access
    std::condition_variable m_condition;           ///< Condition variable for signaling
    std::atomic<bool> m_exit{false};               ///< Atomic flag indicating an exit request
    std::atomic<bool> m_internal_pred_flag{false}; ///< Internal predicate flag used for signaling

    /**
     * @brief Check if an exit request has been made.
     *
     * @return True if an exit is requested, false otherwise.
     */
    bool isExit() const;

    /**
     * @brief Check if the system is currently waiting.
     *
     * This function checks the internal predicate flag to determine if the system
     * is currently waiting for a condition to be met.
     *
     * @return True if waiting, false otherwise.
     */
    bool internalPred() const;

    /**
     * @brief Enable the internal predicate flag.
     *
     * This function sets the internal predicate flag to true, indicating that the system
     * is currently waiting for a condition.
     */
    void enableInternalPred();

    /**
     * @brief Disable the internal predicate flag.
     *
     * This function resets the internal predicate flag to false, indicating that the system
     * is no longer waiting.
     */
    void disableInternalPred();
};

template<typename Pr>
Wait::Status Wait::wait(Pr pred)
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_condition.wait(lock, [this, &pred]() -> bool
                     { return isExit() || pred(); });
    if (isExit())
    {
        return Status::EXIT;
    }
    return Status::SUCCESS;
}

template<class Repr, class Period>
Wait::Status Wait::waitFor(const std::chrono::duration<Repr, Period>& timeout)
{
    enableInternalPred();
    std::unique_lock<std::mutex> lock(m_lock);
    bool status{m_condition.wait_for(lock, timeout, [this]() -> bool
                                     { return isExit() || internalPred(); })};
    if (!status)
    {
        return Status::TIMEOUT;
    }
    if (isExit())
    {
        return Status::EXIT;
    }
    return Status::SUCCESS;
}

template<class Repr, class Period, typename Pr>
Wait::Status Wait::waitFor(const std::chrono::duration<Repr, Period>& timeout, Pr pred)
{
    std::unique_lock<std::mutex> lock(m_lock);
    bool status{m_condition.wait_for(lock, timeout, [this, &pred]() -> bool
                                     { return isExit() || pred(); })};
    if (!status)
    {
        return Status::TIMEOUT;
    }
    if (isExit())
    {
        return Status::EXIT;
    }
    return Status::SUCCESS;
}

} // namespace ThreadSafe
