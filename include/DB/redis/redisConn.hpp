#pragma once
#include "connManager/util.h"
#include "connManager/connManager.h"
#include <cpp_redis/cpp_redis>
#include "heartbeat/heartbeat.h"
using namespace std::placeholders;
template <typename connInfo>
class RedisConn : public std::enable_shared_from_this<RedisConn<connInfo>>
{
  public:
    typedef connInfo conn_type_t;
    RedisConn() = delete;

    RedisConn(connInfo info)
    {
        conn_state = false;
        hb.set_hb_lost_cb([=](int error) {
            this->onDisconnected(info.destIP, std::stoi(info.destPort), cpp_redis::client::connect_state::dropped);
        });

        _info = info;
        __LOG(debug, "new Redis connection! " << (void *)this);
    }
    ~RedisConn() { __LOG(warn, "[RedisConn] ~RedisConn"); }
    void onConnected()
    {
        __LOG(debug, "enter. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
        // set_conn_state(true);
        // tell pool that there is a new connection

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
            this->sync_commit();
        },
                 conn_state);
    }

    void onDisconnected(const std::string &host, std::size_t port, cpp_redis::client::connect_state status)
    {
        if (status == cpp_redis::client::connect_state::dropped)
        {
            __LOG(warn, "onDisconnected. connection id is : " << get_conn_inc_id() << " pool index is : " << get_pool_index());
            set_conn_state(false);
            // tell pool that a connection is deleted
            auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
            bus->call(CONN_DEC, this, get_conn_dec_id());
            client.disconnect(true);
            // just call connect, do somethin later
            connect(_info);
        }
        __LOG(debug, "connect_state is : ");
    }

    bool connect(connInfo info)
    {
        __LOG(debug, "connect to info : ");
        info.dump();
        auto tmp = std::bind(&RedisConn<connInfo>::onDisconnected, this, _1, _2, _3);

        //  typedef std::function<void(const std::string& host, std::size_t port, connect_state status)> connect_callback_t;
        client.connect(info.destIP, std::size_t(std::stoi(info.destPort)), tmp);

        // if the ping is fail? To do

        // get connection status
        this->ping([&](cpp_redis::reply &reply) {
            __LOG(debug, "now send the first PING message!@@@@@@@@@@@@");
            if (reply.is_string())
            {
                if (reply.as_string() == "PONG")
                {
                    conn_state = true;

                    auto bus = message_bus<connPool<RedisConn<connInfo>>>::instance();
                    bus->call(CONN_INC, this, get_conn_inc_id());

                    RedisConn<connInfo>::onConnected();
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
        this->sync_commit();

        return true;
    }

    void get(const std::string &key, const cpp_redis::client::reply_callback_t &reply_callback)
    {
        client.get(key, reply_callback);
    }

    void set(const std::string &key, const std::string &value, const cpp_redis::client::reply_callback_t &reply_callback)
    {
        client.set(key, value, reply_callback);
    }

    void ping(const cpp_redis::client::reply_callback_t &reply_callback)
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
    cpp_redis::client client;
    heartbeat hb;
    std::atomic<bool> conn_state;
};
