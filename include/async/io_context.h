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
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <chrono>


namespace IOService
{
    class io_context
    {
    public:
        class executor_type;
        typedef uint32_t count_type;

        io_context();
        io_context(io_context const &) = delete;
        io_context & operator=(io_context const &) = delete;
        virtual ~io_context() = default;

        executor_type get_executor() noexcept;
        count_type run();
        void stop();
    private:
        boost::asio::io_service _io_service;
        std::unique_ptr<executor_type> _executor;
    };


    class io_context::executor_type
    {
    public:
        executor_type(boost::asio::io_service & io_service);
        executor_type(executor_type const & other) noexcept = default;
        executor_type(executor_type && other) noexcept = default;
        executor_type & operator=(executor_type const & other) noexcept;
        executor_type & operator=(executor_type && other) noexcept;


        template<class Func, class ProtoAllocator = std::allocator<void> >
        void post(Func && f, ProtoAllocator const & a = std::allocator<void>()) const;
        void invoke_later(std::function<void ()> func, std::chrono::milliseconds const & ms);
        void invoke_at(std::function<void ()> func, std::time_t const & date);

    private:
        boost::asio::io_service & _io_service;
    };

    template<class Func, class ProtoAllocator>
    void io_context::executor_type::post(Func && f, ProtoAllocator const & a) const
    {
        _io_service.post(f);
    }
}
