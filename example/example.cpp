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
#include "connManager/messagebus.h"

#include <cpp_redis/cpp_redis>
#include <unistd.h>

int main()
{

    set_log_level(logger_iface::log_level::debug);
    __LOG(error, "hello logger!"
                     << "this is error log");
    __LOG(warn, "hello logger!"
                    << "this is warn log");
    __LOG(info, "hello logger!"
                    << "this is info log");
    __LOG(debug, "hello logger!"
                     << "this is debug log");

    cpp_redis::redis_client client;

    client.connect("127.0.0.1", 6379, [](cpp_redis::redis_client &) {
        std::cout << "client disconnected (disconnection handler)" << std::endl;
    });

    // same as client.send({ "SET", "hello", "42" }, ...)
    client.set("hello", "42", [](cpp_redis::reply &reply) {
        std::cout << "set hello 42: " << reply << std::endl;
        // if (reply.is_string())
        //   do_something_with_string(reply.as_string())
    });

    // same as client.send({ "DECRBY", "hello", 12 }, ...)
    client.decrby("hello", 12, [](cpp_redis::reply &reply) {
        std::cout << "decrby hello 12: " << reply << std::endl;
        // if (reply.is_integer())
        //   do_something_with_integer(reply.as_integer())
    });

    // same as client.send({ "GET", "hello" }, ...)
    client.get("hello", [](cpp_redis::reply &reply) {
        std::cout << "get hello: " << reply << std::endl;
        // if (reply.is_string())
        //   do_something_with_string(reply.as_string())
    });

    client.zadd("zhello", {}, {{"1", "a"}, {"2", "b"}, {"3", "c"}, {"4", "d"}},
                [](cpp_redis::reply &reply) {
                    std::cout << "zadd zhello 1 a 2 b 3 c 4 d: " << reply << std::endl;
                });

    // commands are pipelined and only sent when client.commit() is called
    // client.commit();

    // synchronous commit, no timeout
    client.sync_commit();
    sleep(30);


}
