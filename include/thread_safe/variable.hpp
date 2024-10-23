#pragma once

#include "common.hpp"

#include <mutex>

namespace ThreadSafe
{

#define COMPARISON_OPERATOR_IMPL(op, name)                                                                        \
    template<typename... Args>                                                                                    \
    std::enable_if_t<has_##name##_operator_for_t_class<T, Args...> && std::is_constructible<T, Args&&...>::value, \
                     bool>                                                                                        \
    operator op(Args&&... args)                                                                                   \
    {                                                                                                             \
        std::lock_guard<std::mutex> guard{m_lock};                                                                \
        return (m_value op T(static_cast<Args&&>(args)...));                                                      \
    }

/**
 * @brief A thread-safe wrapper for a variable of type T.
 *
 * This class allows safe concurrent access to a variable by using a mutex
 * to guard its access. It provides thread-safe assignment, comparison, and
 * access to the encapsulated variable.
 *
 * @tparam T The type of the variable to be protected.
 */
template<typename T>
class Variable
{
public:
    using Type = T;

    /**
     * @brief Default constructor.
     *
     * Initializes the variable without any value.
     */
    Variable() = default;

    /**
     * @brief Perfect-forwarding constructor to construct T in place.
     *
     * This constructor allows passing any number of arguments to construct the encapsulated
     * value of type T directly, avoiding extra copies or moves.
     *
     * @tparam Args Variadic template for argument types.
     * @param args Arguments to forward to T's constructor.
     */
    template<typename... Args>
    Variable(Args&&... args)
        : m_value{std::forward<Args>(args)...}
    {
    }

    // Make this class uncopyable
    UNCOPYABLE(Variable);

    /**
     * @brief Thread-safe assignment operator.
     *
     * Assigns a new value to the variable, protected by a mutex lock.
     *
     * @param args The new value to assign.
     */

    template<typename... Args>
    std::enable_if_t<std::is_constructible<T, Args&&...>::value, void> operator=(Args&&... args)
    {
        std::lock_guard<std::mutex> guard{m_lock};
        m_value = T(static_cast<Args&&>(args)...);
    }

    /**
     * @brief Thread-safe comparison operator.
     */
    COMPARISON_OPERATOR_IMPL(==, equal);
    COMPARISON_OPERATOR_IMPL(!=, not_equal);
    COMPARISON_OPERATOR_IMPL(<, less);
    COMPARISON_OPERATOR_IMPL(<=, less_or_equal);
    COMPARISON_OPERATOR_IMPL(>, greater);
    COMPARISON_OPERATOR_IMPL(>=, greater_or_equal);

    /**
     * @brief Thread-safe getter for the value.
     *
     * Returns a copy of the encapsulated value, protected by a mutex lock.
     *
     * @return A copy of the current value.
     */
    T get()
    {
        std::lock_guard<std::mutex> guard{m_lock};
        return m_value;
    }

    /**
     * @brief Thread-safe type conversion operator.
     *
     * Allows the variable to be implicitly converted to its underlying type T.
     *
     * @return A copy of the current value.
     */
    operator T() const
    {
        std::lock_guard<std::mutex> guard{m_lock};
        return m_value;
    }

    /**
     * @brief Invokes a non-member function or callable with the stored value.
     *
     * This function allows invoking a non-member function or callable with the stored value (`m_value`).
     * The function is forwarded as a callable, and the arguments are forwarded as well.
     *
     * @tparam F The type of the callable (function, lambda, etc.)
     * @tparam Args The types of the arguments to the callable.
     * @param func The callable (function, lambda, etc.) to invoke.
     * @param args The arguments to pass to the callable.
     * @return The result of invoking the callable with the stored value and the provided arguments.
     */
    template<typename F,
             typename... Args,
             typename std::enable_if<!std::is_member_function_pointer<F>::value>::type* = nullptr>
    decltype(auto) invoke(F&& func, Args&&... args)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        return std::forward<F>(func)(m_value, std::forward<Args>(args)...);
    }

    /**
     * @brief Invokes a non-const member function on the stored value.
     *
     * This function allows invoking a non-const member function pointer on the stored value (`m_value`).
     * The function pointer and arguments are forwarded appropriately.
     *
     * @tparam R The return type of the member function.
     * @tparam C The type of the class to which the member function belongs.
     * @tparam Args The types of the arguments to the member function.
     * @param func The member function pointer to invoke.
     * @param args The arguments to pass to the member function.
     * @return The result of invoking the member function on the stored value.
     */
    template<typename R, typename C, typename... Args>
    decltype(auto) invoke(R (C::*func)(Args...), Args&&... args)
    {
        std::lock_guard<std::mutex> guard(m_lock);
        return (m_value.*func)(std::forward<Args>(args)...);
    }

    /**
     * @brief Invokes a const member function on the stored value.
     *
     * This function allows invoking a const member function pointer on the stored value (`m_value`).
     * The function pointer and arguments are forwarded appropriately, and the function is invoked on a
     * const value.
     *
     * @tparam R The return type of the member function.
     * @tparam C The type of the class to which the member function belongs.
     * @tparam Args The types of the arguments to the member function.
     * @param func The const member function pointer to invoke.
     * @param args The arguments to pass to the member function.
     * @return The result of invoking the const member function on the stored value.
     */
    template<typename R, typename C, typename... Args>
    decltype(auto) invoke(R (C::*func)(Args...) const, Args&&... args) const
    {
        std::lock_guard<std::mutex> guard(m_lock);
        return (m_value.*func)(std::forward<Args>(args)...);
    }

private:
    T m_value{};                 ///< The encapsulated value of type T.
    mutable std::mutex m_lock{}; ///< A mutex to guard access to m_value.
};

} // namespace ThreadSafe
