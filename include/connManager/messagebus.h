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
#include <string>
#include <map>
#include <unordered_map>
#include <functional>
#include <iostream>
#include <utility>
#include <mutex>
#include <tuple>
#include <thread>

class id_center
{
  public:
    static id_center *instance()
    {
        static auto ins = new id_center();
        return ins;
    }

    int getUniqueID()
    {
        return (uniqueID_atomic++);
    }

  private:
    std::atomic<int> uniqueID_atomic;
};

template <typename OBJ>
class message_bus
{
  public:
    message_bus() { bus_thread_id = std::this_thread::get_id(); }

    static message_bus<OBJ> *instance()
    {
        static auto ins = new message_bus<OBJ>();
        return ins;
    }

    typedef std::function<void(void *, void *)> handler_f;
    int register_handler(std::string const name, void *t, handler_f f)
    {
        if (bus_thread_id != std::this_thread::get_id())
        {
            std::lock_guard<std::mutex> lck(mtx);
        }
        int id = (id_center::instance())->getUniqueID();
        (this->invokers_).emplace(name, std::make_tuple(f, t, id));
        __LOG(debug, "regist handler for " << name << " id is : " << id);
        return id;
    }

    void register_handler(std::string const name, void *t, handler_f f, int id)
    {
        if (bus_thread_id != std::this_thread::get_id())
        {
            std::lock_guard<std::mutex> lck(mtx);
        }
        (this->invokers_).emplace(name, std::make_tuple(f, t, id));
        __LOG(debug, "regist handler for " << name);
    }

    void call(const std::string name, void *message, int id)
    {
        __LOG(debug, "call topic with name : " << name << " message_p is " << (void *)message);
        if (bus_thread_id != std::this_thread::get_id())
        {
            std::lock_guard<std::mutex> lck(mtx);
        }
        auto it = invokers_.begin();
        while (it != invokers_.end())
        {
            if (!name.compare(it->first))
            {
                auto save = it;
                ++save;
                handler_f cb;
                void *obj;
                int _id;
                std::tie(cb, obj, _id) = it->second;
                __LOG(debug, "find topic : " << it->first << " id is : " << id << " _id is : " << _id);
                if (id == _id)
                {
                    __LOG(debug, "find topic : " << it->first << " and call callback. id is : " << id << " _id is : " << _id);
                    cb((void *)obj, message);
                    return;
                }
                it = save;
            }
            else
            {
                it++;
            }
        }
    }

    void remove_handler(std::string const name, void *t, int id)
    {
        __LOG(warn, "remove_handler " << name);
        if (bus_thread_id != std::this_thread::get_id())
        {
            std::lock_guard<std::mutex> lck(mtx);
        }
        auto it = invokers_.begin();
        while (it != invokers_.end())
        {
            if (!name.compare(it->first))
            {
                auto save = it;
                ++save;
                __LOG(debug, "find topic : " << it->first);

                if (std::get<1>(it->second) == t && std::get<2>(it->second) == id)
                {
                    __LOG(debug, "erase one topic : " << it->first);
                    invokers_.erase(it);
                }
                it = save;
            }
            else
            {
                it++;
            }
        }
    }

  private:
    std::unordered_multimap<std::string, std::tuple<handler_f, void *, int>> invokers_;
    std::mutex mtx;
    std::thread::id bus_thread_id;
};
