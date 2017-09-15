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
#include "connManager/util.h"
#include "connManager/serviceDiscovery.h"

// connection pool
template <typename DBConn>
class connPool
{
  public:
    using connInfo = typename DBConn::conn_type_t;
    using DBConn_ptr_t = std::shared_ptr<DBConn>;
    connPool();
    ~connPool();
    bool add_conn(DBConn *conn);
    bool add_conn(connInfo info);
    bool del_conn(DBConn *conn);
    bool del_conn(connInfo info);
    DBConn_ptr_t get_conn();

    void set_id(int id) { _id = id; }
    int get_id() { return _id; }

    int get_conn_inc_id() { return conn_inc_id; }
    int get_conn_dec_id() { return conn_dec_id; }
    void set_conn_inc_id(int id) { conn_inc_id = id; }
    void set_conn_dec_id(int id) { conn_dec_id = id; }

    int get_pool_dec_id() { return pool_dec_id; }
    int get_pool_inc_id() { return pool_inc_id; }
    void set_pool_inc_id(int id) { pool_inc_id = id; }
    void set_pool_dec_id(int id) { pool_dec_id = id; }

  private:
    int _id;
    int conn_inc_id;
    int conn_dec_id;
    int pool_inc_id;
    int pool_dec_id;
    std::list<DBConn_ptr_t> DBInsLocalList;
    std::list<DBConn_ptr_t> DBInsRemoteList;
};

// connmanager
template <typename DBConn, typename serviceDiscovery>
class connManager
{
  public:
    using connInfo = typename DBConn::conn_type_t;
    using pool_ptr_t = std::shared_ptr<connPool<DBConn>>;

    connManager();
    ~connManager();

    void on_unavaliable();
    void on_avaliable();
    int add_pool();
    bool del_pool(int id);
    pool_ptr_t get_pool();

    auto get_conn() -> typename connPool<DBConn>::DBConn_ptr_t;
    bool add_conn(connInfo info);
    bool del_conn(connInfo info);

    void set_pool_dec_id(int id) { pool_dec_id = id; }
    int get_pool_dec_id() { return pool_dec_id; }
    void set_pool_inc_id(int id) { pool_inc_id = id; }
    int get_pool_inc_id() { return pool_inc_id; }

  private:
    std::list<connInfo> infoList; // just used when add a new pool.
    std::list<pool_ptr_t> poolList;
    int pool_inc_id;
    int pool_dec_id;
    serviceDiscovery servDiscov;
};

template <typename DBConn, typename serviceDiscovery>
connManager<DBConn, serviceDiscovery>::connManager()
{
    auto bus = message_bus<connManager<DBConn, void>>::instance();
    set_pool_inc_id(bus->register_handler(POOL_INC, this, [this](void *objp, void *msgp) {
        // this should not happen, but if happened, just add a new pool as there is new connection
        for (auto i : poolList)
        {
            if (i->get_id() == ((connPool<DBConn> *)msgp)->get_id())
            {
                __LOG(debug, "pool : " << i->get_id() << " already exist!");
                return;
            }
        }
        this->add_pool();
    }));
    set_pool_dec_id(bus->register_handler(POOL_DEC, this, [this](void *objp, void *msgp) {
        __LOG(debug, "delete one pool with id : " << ((connPool<DBConn> *)msgp)->get_id());
        this->del_pool(((connPool<DBConn> *)msgp)->get_id());
        __LOG(debug, "now there are " << ((this->poolList).size()) << " pools in the list");
        if (!(this->poolList).size())
        {
            this->on_unavaliable();
        }
    }));

    servDiscov.setOnConnInc([this](connInfo info) -> bool {
        return this->add_conn(info);
    });
    servDiscov.setOnConnDec([this](connInfo info) -> bool {
        return this->del_conn(info);

    });
    servDiscov.setGetConnInfoCb([this]() -> typename serviceDiscovery::connList {
        connInfo info;
        info.destIP = "127.0.0.1";
        info.destPort = "6379";
        info.type = 0;
        typename serviceDiscovery::connList list;
        list.insert(info);
        return list;
    });
}

template <typename DBConn, typename serviceDiscovery>
connManager<DBConn, serviceDiscovery>::~connManager()
{
    auto bus = message_bus<connManager<DBConn, void>>::instance();
    bus->remove_handler(POOL_DEC, this);
    bus->remove_handler(POOL_INC, this);
}

template <typename DBConn, typename serviceDiscovery>
void connManager<DBConn, serviceDiscovery>::on_unavaliable()
{
    __LOG(warn, "on_unavaliable");
}

template <typename DBConn, typename serviceDiscovery>
void connManager<DBConn, serviceDiscovery>::on_avaliable()
{
}

template <typename DBConn, typename serviceDiscovery>
int connManager<DBConn, serviceDiscovery>::add_pool()
{
    pool_ptr_t pool(new connPool<DBConn>());
    for (auto p_info : infoList)
    {
        pool->add_conn(p_info);
    }
    int id = (id_center::instance())->getUniqueID();
    // actually we can resuse one id, just leave it here for further usage
    pool->set_id(id);
    pool->set_pool_dec_id(get_pool_dec_id());
    pool->set_pool_inc_id(get_pool_inc_id());
    poolList.emplace_back(pool);
    __LOG(debug, "add a new pool with id : " << id);
    return id;
}

template <typename DBConn, typename serviceDiscovery>
bool connManager<DBConn, serviceDiscovery>::del_pool(int id)
{
    poolList.remove_if([&](const connManager<DBConn, serviceDiscovery>::pool_ptr_t &ptr_t) -> bool {
        if (ptr_t->get_id() == id)
        {
            return true;
        }
        else
        {
            return false;
        }
    });
    return true;
}

template <typename DBConn, typename serviceDiscovery>
auto connManager<DBConn, serviceDiscovery>::get_pool() -> typename connManager<DBConn, serviceDiscovery>::pool_ptr_t
{
    if (poolList.size())
    {
        auto tmp = poolList.front();
        poolList.pop_front();
        poolList.push_back(tmp);
        return tmp;
    }
    else
    {
        return nullptr;
    }
}

template <typename DBConn, typename serviceDiscovery>
auto connManager<DBConn, serviceDiscovery>::get_conn() -> typename connPool<DBConn>::DBConn_ptr_t
{
    auto tmp = get_pool();
    if (tmp != nullptr)
    {
        __LOG(debug, "get connection from pool " << tmp->get_id());
        return tmp->get_conn();
    }
    else
    {
        return nullptr;
    }
}

template <typename DBConn, typename serviceDiscovery>
bool connManager<DBConn, serviceDiscovery>::add_conn(connInfo info)
{
    infoList.emplace_back(info);
    for (auto tmp : poolList)
    {
        tmp->add_conn(info);
    }
    return true;
}

template <typename DBConn, typename serviceDiscovery>
bool connManager<DBConn, serviceDiscovery>::del_conn(connInfo info)
{
    infoList.remove(info);
    for (auto tmp : poolList)
    {
        __LOG(debug, "connmanager try to delete one connection of pool with id : " << tmp->get_id());
        tmp->del_conn(info);
    }
    return true;
}

// conn pool
template <typename DBConn>
connPool<DBConn>::connPool()
{
    auto bus = message_bus<connPool<DBConn>>::instance();
    int tmp = bus->register_handler(CONN_DEC, this, [this](void *objp, void *msgp) {
        auto conn_ptr_p = (DBConn *)msgp;
        auto obj_p = this;
        if (conn_ptr_p->get_pool_index() == obj_p->get_id())
        {
            __LOG(debug, "conn report disconnected, delete one connection");
            obj_p->del_conn(conn_ptr_p);
            __LOG(warn, " now connection num is : " << ((obj_p->DBInsLocalList).size() + (obj_p->DBInsRemoteList).size()) << " in the pool : " << ((obj_p->get_id())));
            if (((obj_p->DBInsLocalList).size() + (obj_p->DBInsRemoteList).size()) == 0)
            {
                __LOG(warn, " no connection in the pool : " << ((obj_p->get_id())));
                // no connection in the pool then send the POOL_DEC message to the connection manager
                auto bus = message_bus<connManager<DBConn, void>>::instance();
                bus->call(POOL_DEC, this, this->get_pool_dec_id());
            }
        }
    });
    this->set_conn_dec_id(tmp);
    tmp = bus->register_handler(CONN_INC, this, [this](void *objp, void *msgp) {
        __LOG(debug, "entered!!");
        auto conn_ptr_p = (DBConn *)msgp;
        auto obj_p = this;
        if (conn_ptr_p->get_pool_index() == (obj_p->get_id()))
        {
            if (((obj_p->DBInsLocalList).size() + (obj_p->DBInsRemoteList).size()) == 0)
            {
                // this should not happen, when the size is 0, this pool should be deleted and related topic is deleted
                auto bus = message_bus<connManager<DBConn, void>>::instance();
                bus->call(POOL_INC, this, this->get_pool_inc_id());
            }
            obj_p->add_conn(conn_ptr_p);
        }
    });
    this->set_conn_inc_id(tmp);
    __LOG(warn, "conn_inc_id is : " << get_conn_inc_id() << " conn_dec_id is : " << get_conn_dec_id());
}

template <typename DBConn>
connPool<DBConn>::~connPool()
{
    __LOG(warn, "~connPool");
    // should delete all the connection.
    auto bus = message_bus<connPool<DBConn>>::instance();
    bus->remove_handler(CONN_INC, this, get_conn_inc_id());
    bus->remove_handler(CONN_DEC, this, get_conn_dec_id());
}

template <typename DBConn>
bool connPool<DBConn>::add_conn(DBConn *conn)
{
    DBConn_ptr_t DBIns = conn->shared_from_this();
    //(conn->get_conn_info()).dump();
    if (!(conn->get_conn_info()).type)
    {
        __LOG(debug, "add one connection in the local list");
        DBInsLocalList.emplace_back(DBIns);
    }
    else
    {
        __LOG(warn, "add one connection in the remote list");
        DBInsRemoteList.emplace_back(DBIns);
    }
    return true;
}

template <typename DBConn>
bool connPool<DBConn>::add_conn(connInfo info)
{
    //info.dump();
    DBConn_ptr_t DBIns(new DBConn(info));
    DBIns->set_pool_index(_id);
    DBIns->set_conn_dec_id(this->get_conn_dec_id());
    DBIns->set_conn_inc_id(this->get_conn_inc_id());
    return DBIns->connect(info);
    return true;
}

template <typename DBConn>
bool connPool<DBConn>::del_conn(DBConn *conn)
{
    if (!(conn->get_conn_info()).type)
    {
        DBInsLocalList.remove_if([&](const DBConn_ptr_t &db) -> bool {
            if ((void *)(db.get()) == (void *)conn)
            {
                __LOG(warn, "local list : find conn with this pointer : " << (void *)conn);
                return true;
            }
            else
            {
                return false;
            }
        });
    }
    else
    {
        DBInsRemoteList.remove_if([&](const DBConn_ptr_t &db) -> bool {
            if ((void *)(db.get()) == (void *)conn)
            {
                __LOG(warn, "remote list :find conn with this pointer : " << (void *)conn);
                return true;
            }
            else
            {
                return false;
            }
        });
    }
    __LOG(warn, "remove one connection, now local list size is : " << DBInsLocalList.size() << ". remote list size is : " << DBInsRemoteList.size());
    return true;
}

template <typename DBConn>
bool connPool<DBConn>::del_conn(connInfo info)
{
    __LOG(debug, "delete on conn in the : " << ((!info.type) ? "local list" : "remote list"));
    if (!info.type)
    {
        DBInsLocalList.remove_if([&](const DBConn_ptr_t &db) -> bool {

            if (db->get_conn_info() == info)
            {
                __LOG(debug, "find one info in the local list, delete it in the pool : " << this->get_id());
                return true;
            }
            else
            {
                __LOG(debug, "did not find one info in the pool : " << this->get_id());
                return false;
            }
        });
    }
    else
    {
        DBInsRemoteList.remove_if([&](const DBConn_ptr_t &db) -> bool {

            if (db->get_conn_info() == info)
            {
                __LOG(debug, "find one info in the remote list , delete it in the pool : " << this->get_id());
                return true;
            }
            else
            {
                __LOG(debug, "did not find one info in the pool : " << this->get_id());
                return false;
            }
        });
    }
    __LOG(debug, "remove one connection, now local list size is : " << DBInsLocalList.size() << ". remote list size is : " << DBInsRemoteList.size());
    return true;
}

template <typename DBConn>
auto connPool<DBConn>::get_conn() -> typename connPool<DBConn>::DBConn_ptr_t
{
    if (DBInsLocalList.size())
    {
        auto tmp = DBInsLocalList.front();
        DBInsLocalList.pop_front();
        DBInsLocalList.push_back(tmp);
        return tmp;
    }
    else if (DBInsRemoteList.size())
    {
        auto tmp = DBInsRemoteList.front();
        DBInsRemoteList.pop_front();
        DBInsRemoteList.push_back(tmp);
        return tmp;
    }
    else
    {
        __LOG(warn, "no DB instance ");
        return nullptr;
    }
}
// conn interface
#if 0
template <typename connInfo>
class connInterface : public std::enable_shared_from_this<connInterface<connInfo>>
{
  public:
    typedef connInfo conn_type_t;

    connInterface(connInfo info)
    {
        conn_state = false;
        connect(info);
        __LOG(debug, "new connection in the connInterface! " << (void *)this);
    }
    connInterface()= delete;

    virtual ~connInterface()
    {
        //auto bus = message_bus<connInterface<connInfo>>::instance();
        //bus->remove_handler(std::string const &name, void *t);
        disconnect();
    }

    // connect function should do the connect work and start heartbeat timer
    virtual bool connect(connInfo info) = 0;// { return false; };
    virtual bool disconnect() = 0;// { return false; };
    // we need to stop the connection and heartbeat...etc to prepare for kill.
    //virtual void stop() = 0;

    // when the connection is connected or heartbeat success after heartbeat fail, should call this function
    virtual void onConnected() = 0;             // final;
    virtual void onDisconnected(int  debug) = 0; // final;

    connInfo get_conn_info() { return _info; }
    void set_pool_index(int index) { pool_index = index; }
    int get_pool_index() { return pool_index; }
    bool get_conn_state() { return conn_state; }
    void set_conn_state(bool stat) { conn_state = stat; }

    connInfo _info;
    int pool_index;
    bool conn_state;

};
// when the connection is connected or heartbeat success after heartbeat fail, should call this function
template <typename connInfo>
void connInterface<connInfo>::onConnected()
{
    __LOG( debug, "enter");
    conn_state = true;
    // tell pool that there is a new connection
    auto bus = message_bus<connPool<connInterface<connInfo>>>::instance();
    bus->call(CONN_INC, this);
}
template <typename connInfo>
void connInterface<connInfo>::onDisconnected(int  debug)
{
    conn_state = false;
    // tell pool that a connection is deleted
    auto bus = message_bus<connPool<connInterface<connInfo>>>::instance();
    bus->call(CONN_DEC, this);
}
#endif