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

/*
class tmplate:

template <typename connInfo>
class RedisConn : public std::enable_shared_from_this<RedisConn<connInfo>>
{
  public:
    typedef connInfo conn_type_t;
    RedisConn() = delete;

    RedisConn(connInfo info)
    {
        _info = info;
        __LOG(debug, "new Redis connection! " << (void *)this);
    }

    void onConnected()
    {
        __LOG(debug, "enter. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
        set_conn_state(true);
        // tell pool that there is a new connection
        auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
        bus->call(CONN_INC, this, get_conn_inc_id());
    }

    void onDisconnected(int error)
    {
        __LOG(debug, "enter. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
        set_conn_state(false);
        // tell pool that a connection is deleted
        auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
        bus->call(CONN_DEC, this, get_conn_dec_id());
    }

    connInfo get_conn_info() { return _info; }
    void set_pool_index(int index) { pool_index = index; }
    int get_pool_index() { return pool_index; }
    bool get_conn_state() { return conn_state; }
    void set_conn_state(bool stat) { conn_state = stat; }
    void set_conn_inc_id(int id) { conn_inc_id = id; }
    void set_conn_dec_id(int id) { conn_dec_id = id; }
    int get_conn_inc_id() { return conn_inc_id; }
    int get_conn_dec_id() { return conn_dec_id; }

    connInfo _info;
    int pool_index;
    bool conn_state;
    int conn_inc_id;
    int conn_dec_id;

    // function to impelement
    bool connect(connInfo info){}
    bool disconnect() { }
    // write your code here
    
};




 */
#pragma once
#include "connManager/util.h"
#include "connManager/connManager.h"
#include <cpp_redis/cpp_redis>
#include "heartbeat/heartbeat.h"

template <typename connInfo>
class RedisConn : public std::enable_shared_from_this<RedisConn<connInfo>>
{
  public:
    typedef connInfo conn_type_t;
    RedisConn() = delete;

    RedisConn(connInfo info)
    {

        conn_state = false;

        _info = info;
        __LOG(debug, "new Redis connection! " << (void *)this);
    }
    ~RedisConn() { __LOG(warn, "[RedisConn] ~RedisConn"); }
    void onConnected()
    {
        __LOG(debug, "enter. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
        // set_conn_state(true);
        // tell pool that there is a new connection


        // get connection status
        this->ping([&](cpp_redis::reply &reply) {
            __LOG(debug, "now send the PING message!@");
            if (reply.is_string())
            {
                if (reply.as_string() == "PONG")
                {
                    conn_state = true;

                    auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
                    bus->call(CONN_INC, this, get_conn_inc_id());
                    // in the hreatbeat when the connection status is changed do something/.....


                }
                else
                {
                    conn_state = false;
                }
                __LOG(debug, "ping reply : " << reply << std::endl);
            }
            else
            {
                __LOG(warn, "reply is not a string");
            }
        });
        // start heartbeat
        hb.start([this](std::atomic<bool> &flag) {

            this->ping([&](cpp_redis::reply &reply) {
                __LOG(debug, "now send the PING message!@");
                if (reply.is_string())
                {
                    if (reply.as_string() == "PONG")
                    {
                        flag = true;
                    }
                    else
                    {
                        flag = false;
                    }
                    __LOG(debug, "ping reply : " << reply << std::endl);
                }
                else
                {
                    __LOG(warn, "reply is not a string");
                }
            });
#if 0
            this->ping([&](cpp_redis::reply &reply) {
                __LOG(warn, "in the ping function. ping reply : "<<reply);
            });
#endif

            this->sync_commit();

        },
                 conn_state);
    }

    void onDisconnected(int error)
    {
        __LOG(debug, "enter. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
        set_conn_state(false);
        // tell pool that a connection is deleted
        auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
        bus->call(CONN_DEC, this, get_conn_dec_id());
    }

    bool connect(connInfo info)
    {
        __LOG(debug, "connect to info : \n");
        info.dump();
        auto tmp = std::bind(&RedisConn<connInfo>::onDisconnected, this, 1);
        client.connect(info.destIP, std::stoi(info.destPort), tmp);
        RedisConn<connInfo>::onConnected();

        return true;
    }

    void get(const std::string &key, const cpp_redis::redis_client::reply_callback_t &reply_callback)
    {
        client.get(key, reply_callback);
    }

    void set(const std::string &key, const std::string &value, const cpp_redis::redis_client::reply_callback_t &reply_callback)
    {
        client.set(key, value, reply_callback);
    }

    void ping(const cpp_redis::redis_client::reply_callback_t &reply_callback)
    {
        client.ping(reply_callback);
    }
    void sync_commit()
    {
        client.sync_commit();
    }

    bool disconnect()
    {
        return true;
    }
    connInfo get_conn_info()
    {
        return _info;
    }
    void set_pool_index(int index)
    {
        pool_index = index;
    }
    int get_pool_index()
    {
        return pool_index;
    }
    bool get_conn_state()
    {
        return conn_state;
    }
    void set_conn_state(bool stat)
    {
        conn_state = stat;
    }
    void set_conn_inc_id(int id)
    {
        conn_inc_id = id;
    }
    void set_conn_dec_id(int id)
    {
        conn_dec_id = id;
    }
    int get_conn_inc_id()
    {
        return conn_inc_id;
    }
    int get_conn_dec_id()
    {
        return conn_dec_id;
    }

    connInfo _info;
    int pool_index;
    //   bool conn_state;
    int conn_inc_id;
    int conn_dec_id;
    cpp_redis::redis_client client;
    heartbeat hb;
    std::atomic<bool> conn_state;
};
