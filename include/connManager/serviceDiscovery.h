#pragma once
#include <set>
#include "translib/timerManager.h"
#include "translib/timer.h"
#include "logger/logger.hpp"

template <typename connInfo>
class serviceDiscovery
{
  public:
    typedef std::set<connInfo> connList;
    typedef std::function<connList()> getConnFun;
    typedef std::function<bool(connInfo)> onConnInfoChangeCb;
    typedef connInfo connInfo_type_t;

    serviceDiscovery<connInfo>()
    {
        __LOG(debug, "[serviceDiscovery] serviceDiscovery");
        timer = translib::TimerManager::instance()->getTimer();
        interval = 5000;

        timer->startForever(interval, [this]() {

            this->getConnInfo();

        });
    }

    ~serviceDiscovery<connInfo>()
    {
        __LOG(warn, "[serviceDiscovery] ~serviceDiscovery");
    }
    void setGetConnInfoCb(getConnFun get_conn_f)
    {
        _get_conn_f = get_conn_f;
    }

    bool getConnInfo()
    {
        if (_get_conn_f)
        {
            connList tmplist = _get_conn_f();
            for (auto tmp : tmplist)
            {
                if (_conn_list.find(tmp) == _conn_list.end())
                {
                    onConnInfoInc(tmp);
                    _conn_list.insert(tmp);
                    __LOG(warn, "[serviceDiscovery] now there is a new connection. info : ");
                    tmp.dump();
                }
                else
                {
                    __LOG(warn, "[serviceDiscovery] connection info already in the local list : ");
                    tmp.dump();
                }
            }
            for (auto tmp : _conn_list)
            {
                if (tmplist.find(tmp) == tmplist.end())
                {
                    onConnInfoDec(tmp);
                    _conn_list.erase(tmp);
                }
                else
                {
                }
            }
            return true;
        }
        else
        {
            __LOG(warn, " no connection info callback is set");
            return false;
        }
    }

    void onConnInfoInc(connInfo info)
    {
        _cfgInc(info);
    }
    void onConnInfoDec(connInfo info)
    {
        _cfgDec(info);
    }
    void setOnConnInc(onConnInfoChangeCb cfgIncCb)
    {
        _cfgInc = cfgIncCb;
    }
    void setOnConnDec(onConnInfoChangeCb cfgDecCb)
    {
        _cfgDec = cfgDecCb;
    }
    connList _conn_list;
    getConnFun _get_conn_f;
    int interval;
    translib::Timer::ptr_p timer;
    onConnInfoChangeCb _cfgInc;
    onConnInfoChangeCb _cfgDec;
};