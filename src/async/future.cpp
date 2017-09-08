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
#include <async/future.h>
#if 0
namespace IOService
{
    namespace detail
    {
    }

    future<void>::future():
        _shared_state(new detail::future_shared_state<void>)
    {
    }

    IOService::future<void>::future(std::shared_ptr<detail::future_shared_state<void>> const & shared_state)
    {
        _shared_state = shared_state;
    }

    void IOService::future<void>::get()
    {
        if (!_shared_state->_valid)
        {
            std::unique_lock<std::mutex> ul(_shared_state->_mutex);
            _shared_state->_cv.wait(ul);
            _shared_state->_valid = false;
        }
        if (_shared_state->_ex_ptr != nullptr)
        {
            std::rethrow_exception(_shared_state->_ex_ptr);
        }
        
        return;
    }
    

    bool IOService::future<void>::valid() const
    {
        return _shared_state->_valid;
    }

    
    void IOService::detail::future_shared_state<void>::set_value()
    {
        {
            std::lock_guard<std::mutex> lg(_mutex);
            _valid = true;
        }
        notify();
    }

    
    void IOService::detail::future_shared_state<void>::set_exception(std::exception_ptr p)
    {
        {
            std::lock_guard<std::mutex> lg(_mutex);
            _ex_ptr = p;
            _valid = true;
        }
        notify();
    }
    

    
    void IOService::detail::future_shared_state<void>::notify()
    {
        if (_then)
        {
            std::shared_ptr<IOService::detail::future_shared_state<void>> wrapper(this->shared_from_this());
            Scheduler::instance().get_executor().post(
                [=] () mutable
                {
                    _then(std::move(IOService::future<void>(wrapper)));
                    _valid = false;
                } );
        }
        else
        {
            _cv.notify_all();
        }
    }

    


    IOService::future<void> promise<void>::get_future()
    {
        if (2 == _future._shared_state.use_count())
        {

            throw std::future_error(std::make_error_code(std::future_errc::future_already_retrieved));
        }
        IOService::future<void> fut(_future._shared_state);
        return std::move(fut);
    }

    void promise<void>::set_value()
    {
        _future._shared_state->set_value();
    }

    
    void promise<void>::set_exception(std::exception_ptr p)
    {
        _future._shared_state->set_exception(p);
    }


    
    
}
#endif