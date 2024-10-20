#include "thread.hpp"

#ifdef _WIN32
#include <windows.h>
#elif __linux__
#include <pthread.h>
#include <sched.h>
#endif

namespace ThreadSafe
{

constexpr int32_t MAX_NUM_OF_PRIORITY{6};

const NativeThreadPrioritys& defaultNativeThreadPrioritys()
{
#ifdef _WIN32
    static const NativeThreadPrioritys prioritys{{ThreadPriority::LOWEST, THREAD_PRIORITY_LOWEST},
                                                 {ThreadPriority::BELOW_NORMAL, THREAD_PRIORITY_BELOW_NORMAL},
                                                 {ThreadPriority::NORMAL, THREAD_PRIORITY_NORMAL},
                                                 {ThreadPriority::ABOVE_NORMAL, THREAD_PRIORITY_ABOVE_NORMAL},
                                                 {ThreadPriority::HIGHEST, THREAD_PRIORITY_HIGHEST},
                                                 {ThreadPriority::TIME_CRITICAL, THREAD_PRIORITY_TIME_CRITICAL}};
    return prioritys;
#elif __linux__
    constexpr int32_t OFFSET{1}; // Avoid to use exactly min and max priority
    static const int32_t min{::sched_get_priority_min(SCHED_FIFO)};
    static const int32_t max{::sched_get_priority_max(SCHED_FIFO)};
    static const int32_t range{max - min};
    static const NativeThreadPrioritys prioritys{
        {ThreadPriority::LOWEST, min + OFFSET},
        {ThreadPriority::BELOW_NORMAL, min + static_cast<int32_t>(range * 0.25)},
        {ThreadPriority::NORMAL, min + static_cast<int32_t>(range * 0.5)},
        {ThreadPriority::ABOVE_NORMAL, min + static_cast<int32_t>(range * 0.75)},
        {ThreadPriority::HIGHEST, min + static_cast<int32_t>(range * 0.9)},
        {ThreadPriority::TIME_CRITICAL, max - OFFSET}};
    return prioritys;

#endif
};

void setNaitiveThreadPriority(ThreadPriority priority, const std::thread::native_handle_type native_handle)
{
    const NativeThreadPrioritys& default_prioritys{defaultNativeThreadPrioritys()};
#ifdef _WIN32
    ::SetThreadPriority(native_handle, default_prioritys.at(priority));
#elif __linux__

    ::sched_param sch_params;
    sch_params.sched_priority = default_prioritys.at(priority);
    ::pthread_setschedparam(native_handle, SCHED_FIFO, &sch_params);
#endif
}

} // namespace ThreadSafe