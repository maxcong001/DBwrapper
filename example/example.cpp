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
#include "logger/logger.hpp"
#include "connManager/connManager.h"
#include "connManager/connInterface.h"
#include "async/scheduler.h"
#include "connManager/serviceDiscovery.h"
#include "translib/timerManager.h"
#include "translib/timer.h"
//#include "DB/redis/redisConn.hpp"
#include <thread>
#include <unistd.h>

int main()
{
    // setup log related
    set_log_level(logger_iface::log_level::debug);
    // start IO service(this is optional)
    IOService::Scheduler &scheduler = IOService::Scheduler::instance();
    std::thread scheduler_thread([&scheduler] {
        std::cout << "start a new thread to run boost io_service!" << std::endl;
        scheduler.run();
        std::cout << "should not run here" << std::endl;
    });
    // for connection manager(this is optional)
    ConnInfo info;
    info.destIP = "127.0.0.1";
    info.destPort = "6379";
    info.type = 0;
    // new instance
    connManager<RedisConn<ConnInfo>, serviceDiscovery<ConnInfo>> *tmp_comm = new connManager<RedisConn<ConnInfo>, serviceDiscovery<ConnInfo>>();
    tmp_comm->add_pool();
    tmp_comm->add_pool();
    tmp_comm->add_pool();
    // add connection info right now, before service discovery function(this is optional)
    tmp_comm->add_conn(info);
    //#######################################################################################
    // test begin here
    auto timer = translib::TimerManager::instance()->getTimer();
    timer->startRounds(1000, 3, [&]() {
        auto tmp = tmp_comm->get_conn();
        if (tmp == nullptr)
        {
            __LOG(error, "no conn in the list!!");
        }
        else
        {
            __LOG(warn, "got one conection, connection status is " << tmp->get_conn_state());
            tmp->set("hello", "42", [](cpp_redis::reply &reply) {
                __LOG(debug, "set hello 42: " << reply);
            });
            tmp->get("hello", [](cpp_redis::reply &reply) {
                __LOG(debug, "get hello: " << reply);
            });
            tmp->ping([](cpp_redis::reply &reply) {
                __LOG(debug, "get ping reply : " << reply.as_string());
            });
            tmp->sync_commit();
        }
    });
    // wait here
    scheduler_thread.join();
    __LOG(warn, "exit example in 30 secs");
}
