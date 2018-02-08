#include "task_base/include.hpp"
#include "command.hpp"
class worker_task : public task_base
{
  public:
    worker_task(std::string name) : task_base(name)
    {
        _name = name;
    }
    ~worker_task() {}
    virtual bool on_message(TASK_MSG task_msg)
    {
        switch (task_msg.type)
        {
        case MSG_TYPE::TASK_REDIS_PUT:
            std::string command = TASK_ANY_CAST<std::string>(task_msg.body);
            __LOG(error, "get command " << command);
            break;
        case MSG_TYPE::TASK_REDIS_GET:
            break;
        case MSG_TYPE::TASK_REDIS_DEL:
            break;
        case MSG_TYPE::TASK_ADD_CONN:
            break;
        case MSG_TYPE::TASK_DEL_CONN:
            break;
        case MSG_TYPE::TASK_REDIS_PING:
            break;
        default:
            __LOG(warn, "unsupport message type!");
            break;
        }
        return true;
    }
};