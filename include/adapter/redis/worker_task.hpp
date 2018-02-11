#include "task_base/include.hpp"
#include "command.hpp"
#include <signal.h>
#include <hiredis/hiredis.h>
#include <hiredis/async.h>
#include <hiredis/adapters/libevent.h>

void connectCallback(const struct redisAsyncContext *c, int status);
void disconnectCallback(const struct redisAsyncContext *c, int status);
void onMassageCallback(redisAsyncContext *c, void *r, void *privdata);
class worker_task : public task_base
{
  public:
    worker_task(std::string name) : task_base(name)
    {
        _name = name;
        _conn_stat = conn_status::DISCONNECTED;
        _connect_cb = connectCallback;
        _disconnect_cb = disconnectCallback;
    }
    ~worker_task() {}
    virtual bool on_message(TASK_MSG task_msg)
    {
        switch (task_msg.type)
        {
        case MSG_TYPE::TASK_REDIS_PUT:
        {
            std::string command = TASK_ANY_CAST<std::string>(task_msg.body);
            __LOG(debug, "get command " << command);
            redisAsyncCommand(_context, onMassageCallback, NULL, command.c_str());
            break;
        }
        case MSG_TYPE::TASK_REDIS_GET:
            break;
        case MSG_TYPE::TASK_REDIS_DEL:
            break;
        case MSG_TYPE::TASK_REDIS_ADD_CONN:
            process_add_conn(task_msg);
            break;
        case MSG_TYPE::TASK_REDIS_DEL_CONN:
            break;
        case MSG_TYPE::TASK_REDIS_PING:
            break;
#if 0
        case MSG_TYPE::TASK_HB_REQ:
            task_manager::instance()->send2task(TASK0, MSG_TYPE::TASK_HB_RSP, _name, 0);
            break;
#endif
        default:
            __LOG(warn, "unsupport message type!");
            break;
        }
        return true;
    }

    void set_conn_stat(conn_status stat)
    {
        _conn_stat = stat;
    }
    conn_status get_conn_stat()
    {
        return _conn_stat;
    }

    bool process_add_conn(TASK_MSG task_msg)
    {
        add_conn_payload payload = TASK_ANY_CAST<add_conn_payload>(task_msg.body);
        __LOG(error, "connect to : " << payload.ip << ":" << payload.port);
        _conn_stat = conn_status::CONNECTING;

        _context = redisAsyncConnect(payload.ip.c_str(), payload.port);
        if (_context->err)
        {
            __LOG(error, "connect to : " << payload.ip << ":" << payload.port << " return error");
            return false;
        }
        redisLibeventAttach(_context, get_loop());
        // set detail info for worker task, --- remember the context
        set_task_detail_info(_context);

        redisAsyncSetConnectCallback(_context, _connect_cb);
        redisAsyncSetDisconnectCallback(_context, _disconnect_cb);
        return true;
    }

    void set_connect_cb(redisConnectCallback *cb)
    {
        _connect_cb = cb;
    }
    void set_disconnect_cb(redisDisconnectCallback *cb)
    {
        _disconnect_cb = cb;
    }
    conn_status _conn_stat;
    redisAsyncContext *_context;
    redisConnectCallback *_connect_cb;
    redisDisconnectCallback *_disconnect_cb;
};