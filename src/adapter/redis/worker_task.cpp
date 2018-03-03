#include "adapter/redis/worker_task.hpp"

int redisAsyncReconnect(const redisAsyncContext *c)
{
    auto ins = task_manager::instance();
    auto worker_ptr_p = ins->get_task_ptr_p_with_detail_info(c);

    std::string host(c->c.tcp.host);

    int port = c->c.tcp.port;

    void redisAsyncFree(redisAsyncContext * ac);

    redisAsyncContext *new_c = redisAsyncConnect(host.c_str(), port);
    // to do : set source IP
    //std::string source_addr(c->c.tcp.source_addr);
    //redisAsyncContext *redisAsyncConnectBind(const char *ip, int port, const char *source_addr);

    worker_ptr_p->set_task_detail_info(new_c);

    redisLibeventAttach(new_c, worker_ptr_p->get_loop());
    redisAsyncSetConnectCallback(new_c, connectCallback);
    redisAsyncSetDisconnectCallback(new_c, disconnectCallback);

    return REDIS_OK;
}
void connectCallback(const struct redisAsyncContext *c, int status)
{

    if (status != REDIS_OK)
    {
        __LOG(error, "Error: " << c->errstr);
        return;
    }
    auto ins = task_manager::instance();
    auto worker_ptr_p = ins->get_task_ptr_p_with_detail_info(c);
    if (worker_ptr_p)
    {
        auto tmp_worker_ptr_p = std::dynamic_pointer_cast<worker_task>(worker_ptr_p);
        tmp_worker_ptr_p->set_conn_stat(conn_status::CONNECTED);
    }
    else
    {
        __LOG(warn, "there is no worker_ptr_p in the task manager!!!!");
    }
    __LOG(debug, "Connected...\n");
}

void disconnectCallback(const struct redisAsyncContext *c, int status)
{
    __LOG(warn, "disConnected...");
    if (status != REDIS_OK)
    {
        __LOG(error, "Error: " << c->errstr);
        return;
    }
    auto ins = task_manager::instance();
    auto worker_ptr_p = ins->get_task_ptr_p_with_detail_info(c);
    if (worker_ptr_p)
    {
        auto tmp_worker_ptr_p = std::dynamic_pointer_cast<worker_task>(worker_ptr_p);
        tmp_worker_ptr_p->set_conn_stat(conn_status::DISCONNECTED);

        // start a reconnect timer
        // to do : add change timer API
        tmp_worker_ptr_p->getTimer()->startOnce(5000, [=]() {
            if (redisAsyncReconnect(c) != REDIS_OK)
            {
                __LOG(warn, "reconnect fail...");
            }
            else
            {
                __LOG(warn, "reconnect ...");
            }

        });
    }
    else
    {
        __LOG(warn, "there is no worker_ptr_p in the task manager!!!!");
        // to do : tell the manager to delete or restart the task
        // just reconnect here

        if (redisAsyncReconnect(c) != REDIS_OK)
        {
            __LOG(warn, "reconnect fail...");
        }
        else
        {
            __LOG(warn, "reconnect ...");
        }
    }
}
void onMassageCallback(redisAsyncContext *c, void *r, void *privdata)
{

    redisReply *reply = (redisReply *)r;
    if (reply == NULL)
    {
        return;
    }
    __LOG(warn, "get message back: private data ptr : " << (void *)privdata << " data is " << reply->str);
}
