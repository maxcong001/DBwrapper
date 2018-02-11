#include "adapter/redis/worker_task.hpp"

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
    }
    else
    {
        __LOG(warn, "there is no worker_ptr_p in the task manager!!!!");
    }
    __LOG(warn, "disConnected...\n");
}
void onMassageCallback(redisAsyncContext *c, void *r, void *privdata)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL)
        return;
    __LOG(warn, "get message back: private data ptr : " << (void *)privdata << " data is " << reply->str);
}