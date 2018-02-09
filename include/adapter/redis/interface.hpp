#include "util.hpp"
#include "worker_task.hpp"

class redis_async_client
{
  public:
    redis_async_client() = default;
    void init()
    {
        ins = task_manager::instance();
        // add worker task
        std::shared_ptr<task_base> worker_task_ptr_p = std::shared_ptr<worker_task>(new worker_task(std::string(WORKER001)));
        ins->add_tasks(worker_task_ptr_p);
        // init the task manager, this will start the manager task and HB
        // no blocking API
        ins->init(false);
    }
    void put()
    {
        put_command_container cmd_container;
        cmd_container.type = MSG_TYPE::TASK_REDIS_PUT;
        cmd_container.key = std::string("test_key");
        cmd_container.msg_value = std::string("test_value");

        std::string command2send = redis_command<put_command_container>::get_command(cmd_container);
        ins->send2task(WORKER001, MSG_TYPE::TASK_REDIS_PUT, command2send);
    }
    task_manager *ins;
};
