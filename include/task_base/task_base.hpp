#pragma once
#include <task_base/util.hpp>
void evfdCallback(int fd, short event, void *args);
class task_base
{
  public:
    task_base(std::string name) : _timer_mgr(_loop)
    {
        _hb_itval = TASK_HB_INTERVAL;
        _name = name;
        _evfd = -1;
    }
    task_base() = delete;
    virtual ~task_base()
    {
        if (_evfd >= 0)
        {
            close(_evfd);
            _evfd = -1;
        }
    }
    virtual void restart()
    {
        // to do
    }

    virtual bool on_before_loop()
    {
        return true;
    }
    virtual bool on_after_loop()
    {
        __LOG(warn, "this is default funtion");
        return true;
    }
    // set the callback function for evnet coming
    virtual bool on_message(TASK_MSG msg) = 0;
    bool init(bool new_thread)
    {
        _evfd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
        if (_evfd < 0)
        {
            __LOG(error, "!!!!!!!!create event fd fail!");
            return false;
        }
        __LOG(debug, "init task with ID :" << _evfd);
        // start eventfd server
        try
        {
            _event_server = std::make_shared<translib::EventFdServer>(_loop.ev(), _evfd, evfdCallback, this);
        }
        catch (std::exception &e)
        {
            __LOG(error, "!!!!!!!!!!!!exception happend when trying to create event fd server, info :" << e.what());
            return false;
        }
        on_before_loop();
        _loop.start(new_thread);
        return true;
    }
    void process_msg(uint64_t num);
    void in_queue(TASK_MSG msg)
    {
        //__LOG(debug, "inqueue for task with id :" << _evfd);
        std::lock_guard<std::mutex> lck(mtx);
        _task_queue.emplace(msg);
    }
    int get_id()
    {
        return _evfd;
    }
    translib::Loop &get_loop()
    {
        return _loop;
    }

    std::string get_task_name()
    {
        return _name;
    }
    virtual void set_hb_interval(std::uint32_t interval)
    {
        _hb_itval = interval;
    }
    std::uint32_t _hb_itval;
    std::mutex mtx;
    TASK_QUEUE _task_queue;
    TASK_QUEUE _tmp_task_queue;
    // task name
    std::string _name;
    int _evfd;
    // note: do not change the sequence of _loop and _event_server
    // _event_server should distructure first!!!!
    translib::Loop _loop;
    std::shared_ptr<translib::EventFdServer> _event_server;
    // timer.
    translib::TimerManager _timer_mgr;
};
