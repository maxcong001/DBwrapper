#include "task_base/task_base.hpp"
#include <task_base/task_manager.hpp>
void evfdCallback(int fd, short event, void *args)
{
    uint64_t one;
    int ret = read(fd, &one, sizeof one);
    if (ret != sizeof one)
    {
        __LOG(warn, "read return : " << ret);
        return;
    }
    //__LOG(debug, "task with id : " << fd << " receive eventfd message, count is : " << one);
    // for (uint64_t i = 0; i < one; i++)
    // {
    task_base *tmp = reinterpret_cast<task_base *>(args);
    tmp->process_msg(one);
    // }
}
//!!!!NOTE, please make sure your job use less time than heartbeat interval,
// or we will consider the heart beat is lost!!!!!
void task_base::process_msg(uint64_t num)
{
    //__LOG(debug, "task with id : " << _evfd << " receive message");
    {
        std::lock_guard<std::mutex> lck(mtx);
        // actually process all the messages
        swap(_task_queue, _tmp_task_queue);
    }
    while (_tmp_task_queue.size() != 0)
    {
        auto tmp = _tmp_task_queue.front();

        if (tmp.type == MSG_TYPE::TASK_HB_REQ)
        {

            if (!_name.compare(TASK0))
            {
                _tmp_task_queue.pop();
                on_message(tmp);
                 // this is task 0, do not send HB rsp
                continue;
            }
            else
            {
                _tmp_task_queue.pop();
                // this is HB message, send rsp
                __LOG(debug, "task : " << _name << " send HB response to task0");
                tmp.body = _name;
                tmp.type = MSG_TYPE::TASK_HB_RSP;
                task_manager::instance()->send2task(TASK0, tmp);
                continue;
            }
        }

        on_message(tmp);
        _tmp_task_queue.pop();
    }
}
