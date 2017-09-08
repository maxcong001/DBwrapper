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

#include <memory>
#include <chrono>
#include <async/io_context.h>

namespace IOService
{

    class Scheduler: public io_context
    {
    public:
        ~Scheduler() = default;

        static Scheduler & instance();
        void destroy();

    protected:
        Scheduler();

    private:
        static std::unique_ptr<Scheduler> _instance;
    };


    inline void invoke_now(std::function<void ()> func)
    {
        Scheduler::instance().get_executor().post(func);
    }

    inline void invoke_later(std::function<void ()> func, std::chrono::milliseconds const & ms)
    {
        Scheduler::instance().get_executor().invoke_later(func, ms);
    }


    inline void invoke_later(std::function<void ()> func, uint32_t ms)
    {
        invoke_later(func, std::chrono::milliseconds(ms));
    }


    inline void invoke_at(std::function<void ()> func, std::time_t const & date)
    {
        Scheduler::instance().get_executor().invoke_at(func, date);
    }

}
