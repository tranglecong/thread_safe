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
    return m_exit;
}

bool Wait::internalPred() const
{
    return !m_internal_pred_flag;
}

void Wait::enableInternalPred()
{
    if (!m_internal_pred_flag)
    {
        m_internal_pred_flag = true;
    }
}

void Wait::disableInternalPred()
{
    if (m_internal_pred_flag)
    {
        m_internal_pred_flag = false;
    }
}

void Wait::exit()
{
    m_exit = true;
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