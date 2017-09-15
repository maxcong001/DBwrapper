/*
 * Copyright (c) 2016-20017 Max Cong <savagecm@qq.com>
 * this code can be found at https://github.com/maxcong001/connection_manager
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
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
        //this->getConnInfo();
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
                    __LOG(debug, "[serviceDiscovery] connection info already in the local list : ");
                    tmp.dump();
                }
            }
            for (auto tmp : _conn_list)
            {
                if (tmplist.find(tmp) == tmplist.end())
                {
                    onConnInfoDec(tmp);
                    __LOG(warn, "[serviceDiscovery] now there is a connection to delete. info : ");
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