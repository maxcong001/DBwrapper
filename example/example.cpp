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
#include "adapter/redis/interface.hpp"
void connectCallback(const struct redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        __LOG(error, "Error: " << c->errstr);
        return;
    }
    __LOG(debug, "Connected...\n");
}
void disconnectCallback(const struct redisAsyncContext *c, int status)
{
    if (status != REDIS_OK)
    {
        __LOG(error, "Error: " << c->errstr);
        return;
    }
    __LOG(warn, "disConnected...\n");
}
void on_message_arrive_cb(struct redisAsyncContext *ac, void *r, void *usr_data)
{
    redisReply *reply = (redisReply *)r;
    if (reply == NULL)
    {
        return;
    }
    __LOG(warn, "get message back: usr_data ptr : " << (void *)usr_data << ". Data is " << reply->str);
}

int main()
{
    // setup log related
    set_log_level(logger_iface::log_level::debug);
    redis_async_client client;

    client.init();
    client.add_conn("127.0.0.1", 6379);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client.put("test_key", "test_value", NULL, on_message_arrive_cb);

    client.send_raw_command("SET test_raw_key test_raw_value", NULL, on_message_arrive_cb);
    std::this_thread::sleep_for(std::chrono::seconds(20));
    __LOG(warn, "exit example in 20 secs");
}
