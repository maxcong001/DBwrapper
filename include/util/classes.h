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
#include <util/function_traits.h>

namespace util
{
    class NonCopyable
    {
    public:
        NonCopyable(NonCopyable const &) = delete;
        NonCopyable & operator=(NonCopyable const &) = delete;
    protected:
        NonCopyable() = default;
    };



    /**
     * Wraps the given (possibly) NonCopyable object and provides a Copyable object.
     */
    template <typename T>
    struct make_copyable
    {
        make_copyable() = default;
        
        make_copyable(make_copyable<T> const & other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable(make_copyable<T> & other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable & operator=(make_copyable<T> const & other)
        {
            _instance = std::move(other._instance);
        }
        
        make_copyable(make_copyable<T> && other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable & operator=(make_copyable<T> && other)
        {
            _instance = std::move(other._instance);
        }
        

        template <typename U>
        make_copyable(U && u):
            _instance(std::move(u))
        {
        }
        
        T && operator*()
        {
            return std::move(_instance);
        }
        
        T && operator*() const
        {
            return std::move(_instance);
        }

    protected:
        mutable T _instance;
    };


    
    template <typename T>
    struct make_copyable_function
    {
        make_copyable_function() = default;
        
        make_copyable_function(make_copyable_function<T> const & other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable_function(make_copyable_function<T> & other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable_function & operator=(make_copyable_function<T> const & other)
        {
            _instance = std::move(other._instance);
        }
        
        make_copyable_function(make_copyable_function<T> && other):
            _instance(std::move(other._instance))
        {
        }

        make_copyable_function & operator=(make_copyable_function<T> && other)
        {
            _instance = std::move(other._instance);
        }
        

        template <typename U>
        make_copyable_function(U && u):
            _instance(std::move(u))
        {
        }
        
        T && operator*()
        {
            return std::move(_instance);
        }
        
        T && operator*() const
        {
            return std::move(_instance);
        }


        template <typename... Args>
        typename util::function_traits<T(Args...)>::return_type operator()(Args... args)
        {
            return _instance(std::forward<Args>(args)...);
        }

        template <typename... Args>
        typename util::function_traits<T(Args...)>::return_type operator()(Args... args) const
        {
            return _instance(std::forward<Args>(args)...);
        }

    protected:
        mutable T _instance;
    };
}
