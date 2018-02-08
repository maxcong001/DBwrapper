#pragma once
#include <task_base/util.hpp>
#include <task_base/task_base.hpp>

class manager_task : public task_base
{

  public:
    manager_task(std::string name) : task_base(name)
    {
        _name = TASK0;
        _hb_tid = 0;
    }

    ~manager_task()
    {
    }
    void restart() override
    {
        // to do
    }
    void set_hb_interval(std::uint32_t interval) override
    {
        _hb_itval = interval;
        _timer_mgr.killTimer(_hb_tid);
        start_hb();
    }
    bool on_before_loop() override;

    bool on_message(TASK_MSG msg) override;

    bool start_hb();

    std::map<std::string, bool> hb_map;
    // heart beat timer id
    int _hb_tid;
};

typedef std::shared_ptr<task_base> task_ptr_t;

class task_manager
{
  public:
    task_manager() : _hb_itval(1000), _seq_id(0)
    {
    }
    static task_manager *instance()
    {
        static task_manager *ins = new task_manager();
        return ins;
    }
    bool send2task(std::string name, MSG_TYPE type, TASK_ANY body, std::uint32_t seq_id = 0)
    {
        // actually here need a lock here
        // but if you start and tasks and do not add more tasks
        // the lock is not needed
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return false;
        }
        TASK_MSG msg;
        msg.type = type;
        msg.body = body;
        msg.seq_id = seq_id;
        it->second->in_queue(msg);
        // send eventfd message
        uint64_t one = 1;
        int ret = write(it->second->get_id(), &one, sizeof(one));
        if (ret != sizeof(one))
        {
            __LOG(error, "write event fd : " << it->second->get_id() << " fail");
            return false;
        }
        else
        {
            //__LOG(debug, "send to eventfd : " << it->second->get_id());
        }
        return true;
    }
    bool send2task(std::string name, TASK_MSG msg)
    {
        // actually here need a lock here
        // but if you start and tasks and do not add more tasks
        // the lock is not needed
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return false;
        }

        it->second->in_queue(msg);
        // send eventfd message
        uint64_t one = 1;
        int ret = write(it->second->get_id(), &one, sizeof(one));
        if (ret != sizeof(one))
        {
            __LOG(error, "write event fd : " << it->second->get_id() << " fail");
            return false;
        }
        else
        {
            //__LOG(debug, "send to eventfd : " << it->second->get_id());
        }
        return true;
    }
    // this should be called by taask0
    // send HB message with body equals to remote task name
    bool send_hb_all()
    {
        for (auto it : task_map)
        {
            if (it.first.compare(TASK0))
            {
                // not task0
                send2task(it.first, MSG_TYPE::TASK_HB, it.first, _seq_id);
                _seq_id++;
            }
            else
            {
            }
        }
        return true;
    }
    bool add_tasks(task_ptr_t task)
    {
        // no lock needed here, this is called only
        // when init
        // if add task at runtime, then need lock here
        std::string task_name = task->get_task_name();
        task_map[task_name] = task;
        return true;
    }
    bool del_tasks(std::string name)
    {
        task_map.erase(name);
        return true;
    }
    int get_task_id(std::string name)
    {
        auto it = task_map.find(name);
        if (it == task_map.end())
        {
            __LOG(warn, "no such a task named : " << name);
            return -1;
        }
        return it->second->get_id();
    }

    // note  if _poll is set to true, it will hang here and wait for incoming message
    bool init(bool _poll = true)
    {
        // add task0
        //std::shared_ptr<task_base> tmp_task_ptr_t = std::shared_ptr<manager_task>(new manager_task(std::string(TASK0)));
        task_ptr_t tmp_task_ptr_t = std::static_pointer_cast<task_base>(std::make_shared<manager_task>(std::string(TASK0)));
        add_tasks(tmp_task_ptr_t);

        if (task_map.find(TASK0) == task_map.end())
        {
            __LOG(error, "!!!!!!!!!at lease task0 should be provided!!");
            return false;
        }

        for (auto it : task_map)
        {
            if (!it.first.compare(TASK0))
            {
            }
            else
            {
                // this is not task 0, start a new task
                it.second->init(true);
            }
        }
        // init the task0
        if (_poll)
        {
            task_map[TASK0]->set_hb_interval(_hb_itval);
            task_map[TASK0]->init(false);
        }
        else
        {
            task_map[TASK0]->set_hb_interval(_hb_itval);
            task_map[TASK0]->init(true);
        }

        return true;
    }
    void set_hb_interval(std::uint32_t _hb_itval)
    {
        _hb_itval = _hb_itval;
        task_map[TASK0]->set_hb_interval(_hb_itval);
    }
    std::map<std::string, task_ptr_t> task_map;
    // heart beat interval
    std::uint32_t _hb_itval;
    std::atomic<std::uint32_t> _seq_id;
};
