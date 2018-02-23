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
    bool put(std::string key, std::string value, void *usr_data, redisCallbackFn *fn)
    {
        std::string command2send = redis_command<std::string, std::string>::get_format_command(MSG_TYPE::TASK_REDIS_PUT, key, value);

        __LOG(debug, "get command :\n"
                         << command2send);
        return send_format_raw_command(command2send, usr_data, fn);
    }
    bool add_conn(std::string ip, int port)
    {
        add_conn_payload add_cmd;
        add_cmd.ip = ip;
        add_cmd.port = port;
        ins->send2task(WORKER001, MSG_TYPE::TASK_REDIS_ADD_CONN, add_cmd);
        return true;
    }
    bool send_format_raw_command(std::string command, void *usr_data, redisCallbackFn *fn)
    {
        TASK_REDIS_FORMAT_RAW_MSG msg;
        msg.cb = fn;
        msg.body = command;
        msg.usr_data = usr_data;
        ins->send2task(WORKER001, MSG_TYPE::TASK_REDIS_FORMAT_RAW, msg);
        return true;
    }
    bool send_raw_command(std::string command, void *usr_data, redisCallbackFn *fn)
    {
        TASK_REDIS_RAW_MSG msg;
        msg.cb = fn;
        msg.body = command;
        msg.usr_data = usr_data;
        ins->send2task(WORKER001, MSG_TYPE::TASK_REDIS_RAW, msg);
        return true;
    }
    task_manager *ins;
};
