#include "wait.hpp"

namespace ThreadSafe
{

Wait::~Wait()
{
    exit();
}

void Wait::notify()
{
    disableInternalPred();
    m_condition.notify_all();
}

bool Wait::isExit() const
{
    return m_exit.load(std::memory_order_acquire);
}

bool Wait::internalPred() const
{
    return !m_internal_pred_flag.load(std::memory_order_acquire);
}

void Wait::enableInternalPred()
{
    if (!m_internal_pred_flag.load(std::memory_order_acquire))
    {
        m_internal_pred_flag.store(true, std::memory_order_release);
    }
}

void Wait::disableInternalPred()
{
    if (m_internal_pred_flag.load(std::memory_order_acquire))
    {
        m_internal_pred_flag.store(false, std::memory_order_release);
    }
}

void Wait::exit()
{
    m_exit.store(true, std::memory_order_release);
    m_condition.notify_all();
}

Wait::Status Wait::wait()
{
    enableInternalPred();
    std::unique_lock<std::mutex> lock(m_lock);
    m_condition.wait(lock, [this]() -> bool
                     { return isExit() || internalPred(); });
    if (isExit())
    {
        return Status::EXIT;
    }
    return Status::SUCCESS;
}

} // namespace ThreadSafe