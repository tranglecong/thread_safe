#pragma once
#include <type_traits>

#define UNCOPYABLE(classname)                        \
    classname(const classname&) = delete;            \
    classname& operator=(const classname&) = delete; \
    classname(classname&&) = delete;                 \
    classname& operator=(classname&&) = delete;

#define ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(op, name)                                                             \
    template<typename T, typename... Args>                                                                           \
    auto t_class_support_##name##_operator(int)->decltype(std::declval<T>() op std::declval<T>(), std::true_type{}); \
    template<typename, typename...>                                                                                  \
    auto t_class_support_##name##_operator(...)->std::false_type;                                                    \
    template<typename T, typename... Args>                                                                           \
    inline constexpr bool has_##name##_operator_for_t_class =                                                        \
        decltype(t_class_support_##name##_operator<T, Args...>(0))::value;

ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(==, equal);
ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(!=, not_equal);
ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(<, less);
ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(<=, less_or_equal);
ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(>, greater);
ADD_HAS_COMPARISON_OPERATOR_TYPE_CHECK(>=, greater_or_equal);