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
#include <async/io_context.h>
#include <future>
#include <iostream>
#include <ctime>


using namespace IOService;
using namespace std;


io_context::io_context():
    _executor(new io_context::executor_type(_io_service))
{}


io_context::executor_type io_context::get_executor() noexcept
{
    return *_executor;
}

io_context::count_type io_context::run()
{
    _io_service.reset();
    boost::asio::io_service::work work(_io_service);
    return _io_service.run();
}

void io_context::stop()
{
    _io_service.stop();
}


io_context::executor_type::executor_type(boost::asio::io_service & io_service)
    : _io_service(io_service)
{}


void io_context::executor_type::invoke_later(std::function<void ()> func, std::chrono::milliseconds const & ms)
{
    std::shared_ptr<boost::asio::deadline_timer> t(new boost::asio::deadline_timer(_io_service, boost::posix_time::milliseconds(ms.count())));
    auto lambda = [func, t] (boost::system::error_code const &) -> void // Make a copy from deadline_timer
        {
            func();
        };
    t->async_wait(lambda);
}


void io_context::executor_type::invoke_at(std::function<void ()> func, std::time_t const & date)
{
    std::shared_ptr<boost::asio::deadline_timer> t(new boost::asio::deadline_timer(_io_service));
    auto lambda = [func, t] (boost::system::error_code const &) -> void // Make a copy from deadline_timer
        {
            func();
        };
    t->expires_at(boost::posix_time::from_time_t(date));
    t->async_wait(lambda);
}
