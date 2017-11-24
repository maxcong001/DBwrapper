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
#include "DB/redis/redisConn.hpp"
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
