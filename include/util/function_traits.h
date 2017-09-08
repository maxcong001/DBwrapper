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

#include <type_traits>
#include <utility>
#include <memory>

namespace util
{
    template<class F>
    struct function_traits;

// Function pointers
// std::string (*)()    
    template<class R>
    struct function_traits<R(*)()>
    {
        using return_type = R;
 
        static constexpr std::size_t arity = 0;
    };


    template<class R, typename... Args>
    struct function_traits<R(*)(Args...)>
    {
        using return_type = R;
 
        static constexpr std::size_t arity = 0;
    };
    
// Functors
// std::string ()(Args...)
    template<class R, class... Args>
    struct function_traits<R(Args...)>
    {
        using return_type = typename std::result_of<R(Args...)>::type;
 
        static constexpr std::size_t arity = sizeof...(Args);
 
        template <std::size_t N>
        struct argument
        {
            static_assert(N < arity, "error: invalid parameter index.");
            using type = typename std::tuple_element<N,std::tuple<Args...>>::type;
        };
    };


// Member function pointers
// std::string (A::*)(Args...)
    template<class C, class R, class... Args>
    struct function_traits<R(C::*)(Args...)>
    {
        using return_type = R;
    };
 
// Const member function pointers
// std::string (A::*)(Args...) const
    template<class C, class R, class... Args>
    struct function_traits<R(C::*)(Args...) const>
    {
        using return_type = R;
    };
 

    template<class F>
    struct function_traits<F&> : public function_traits<F>
    {};
 
    template<class F>
    struct function_traits<F&&> : public function_traits<F>
    {};
  
}
