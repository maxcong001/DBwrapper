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
#if 0
#include <async/scheduler.h>

#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <utility>
#include <util/function_traits.h>
#include <util/classes.h>

namespace IOService
{
    template <typename T>
    class promise;
    template <typename T>
    class future;
    template <typename Func>
    typename std::result_of<Func()>::type run_now(Func && func);


    template <typename T, typename S = void>
    struct is_future;

    template <typename S>
    struct is_future<IOService::future<S>>: public std::true_type
    {};

    template <typename T>
    struct is_future<T>: public std::false_type
    {};

    namespace detail
    {
        template <typename T>
        struct future_shared_state;

        template <typename T>
        struct future_shared_state: public std::enable_shared_from_this<future_shared_state<T>>
        {
            T _value;
            std::exception_ptr _ex_ptr;
            bool _valid;

            std::function<void (IOService::future<T> &&)> _then;

            std::mutex _mutex;
            std::condition_variable _cv;
            future_shared_state(): _valid(false), _then{nullptr} {}

            void set_value(T const & value);
            void set_value(T && value);
            void set_value(T & value);
            void set_value(future_shared_state<T> const & other)
            {
                set_value(other._value);
            }

            void set_exception(std::exception_ptr p);
            void notify();

        };

        template <>
        struct future_shared_state<void>: public std::enable_shared_from_this<future_shared_state<void>>
        {
            std::exception_ptr _ex_ptr;
            bool _valid;

            std::function<void (IOService::future<void> &&)> _then;

            std::mutex _mutex;
            std::condition_variable _cv;

            future_shared_state(): _valid(false), _then{nullptr} {}

            void set_value();
            void set_value(future_shared_state<void> const & other)
            {
                set_value();
            }
            void set_exception(std::exception_ptr p);
            void notify();
        };


        template <bool is_return_type_a_future, typename R>
        struct then_functor;

        /*
         * Current future is C = future<T> // T might be void
         * Return type of then-function is R = future<S>
         */
        template <typename R>
        struct then_functor<true, R>
        {
            then_functor() = default;

            template <typename Func, typename C>
            R operator()(Func && copyable_func, C && current_future)
            {
                auto _shared_state = current_future._shared_state;
                std::lock_guard<std::mutex> lg(_shared_state->_mutex);
                using Func_Return_Type = R;
                using Func_Future_Value_Type = typename Func_Return_Type::value_type;
                using Current_Future_Plain = typename std::remove_reference<C>::type;
                using Current_Future_RRef = Current_Future_Plain &&;

                Func_Return_Type then_future;
                std::shared_ptr<detail::future_shared_state<Func_Future_Value_Type>> shared_state_of_then_future = then_future._shared_state;

                std::function<Func_Return_Type (Current_Future_RRef)> wrapped_func =
                    [shared_state_of_then_future, copyable_func] (Current_Future_RRef current_future) -> Func_Return_Type
                    {
                        Func_Return_Type future_of_function;
                        try
                        {
                            future_of_function = std::move(copyable_func(std::move(current_future)));
                        }
                        catch (...)
                        {
                            // Exception was thrown in the function which was invoked with then().
                            future_of_function._shared_state->set_exception(std::current_exception());
                        }
                        // It's guaranteed that the future_of_function haven't got then() function neither has ongoing get() execution!
                        // But it can be valid now!
                        // ...and here comes the magic!
                        detail::future_shared_state<Func_Future_Value_Type> * shared_state_of_future_of_function = future_of_function._shared_state.get();

                        shared_state_of_future_of_function->_then =
                        [shared_state_of_future_of_function, shared_state_of_then_future] (Func_Return_Type && fut)
                        {
                            if (shared_state_of_future_of_function->_ex_ptr)
                            {
                                // passing the exception to the then_future. No need to lock its shared state, then set_exception() will do that.
                                shared_state_of_then_future->set_exception(shared_state_of_future_of_function->_ex_ptr);
                            }
                            else // There weren't any exceptions, so value has been set
                            {
                                // passing the value to the then_future. No need to lock it, the set_value will do it.
                                shared_state_of_then_future->set_value(*shared_state_of_future_of_function);
                            }
                        };
                        // If future_of_function is already valid, call notify().
                        if (future_of_function.valid())
                        {
                            shared_state_of_future_of_function->notify();
                        }
                        return std::move(future_of_function);
                    };

                _shared_state->_then = wrapped_func;
                // then function is added to the future.
                // If set_value/set_exception has already called before this then() called, start notification.
                if (_shared_state->_valid)
                {
                    _shared_state->notify();
                }
                return std::move(then_future);
            }
        };


        /*
         * Current future is future<C>
         * Return type of then-function is not future (R, can be void).
         * No need to pass-by the then-wrapped-function's result to the then().
         */
        template <typename R>
        struct then_functor<false, R>
        {
            then_functor() = default;

            template <typename Func, typename C>
            R operator()(Func && copyable_func, C && current_future)
            {
                auto _shared_state = current_future._shared_state;
                std::lock_guard<std::mutex> lg(_shared_state->_mutex);
                _shared_state->_then = copyable_func;

                if (_shared_state->_valid)
                {
                    _shared_state->notify();
                }
                return R();
            }
        };


        template <typename T>
        void IOService::detail::future_shared_state<T>::set_value(T const & value)
        {
            {
                std::lock_guard<std::mutex> lg(_mutex);
                _value = value;
                _valid = true;
            }
            notify();
        }

        template <typename T>
        void IOService::detail::future_shared_state<T>::set_value(T && value)
        {
            {
                std::lock_guard<std::mutex> lg(_mutex);
                _value = std::move(value);
                _valid = true;
            }
            notify();
        }

        template <typename T>
        void IOService::detail::future_shared_state<T>::set_value(T & value)
        {
            {
                std::lock_guard<std::mutex> lg(_mutex);
                _value = value;
                _valid = true;
            }
            notify();
        }

        template <typename T>
        void IOService::detail::future_shared_state<T>::set_exception(std::exception_ptr p)
        {
            {
                std::lock_guard<std::mutex> lg(_mutex);
                _ex_ptr = p;
                _valid = true;
            }
            notify();
        }

        template <typename T>
        void IOService::detail::future_shared_state<T>::notify()
        {
            if (_then)
            {
                std::shared_ptr<IOService::detail::future_shared_state<T>> wrapper(this->shared_from_this());
                Scheduler::instance().get_executor().post(
                    [=] () mutable
                    {
                        _then(std::move(IOService::future<T>(wrapper)));
                        _valid = false;
                    } );
            }
            else
            {
                _cv.notify_all();
            }
        }

    } // end of IOService::detail namespace



    /*
     * future class for non-void types
     */
    template <typename T>
    class future
    {
    public:
        using value_type = T ;

        future();
        future(future && other) = default;
        future(future const & other) = delete;
        future & operator=(future && other) = default;
        future & operator=(future const & other) = delete;
        ~future() = default;

        T get();
        bool valid() const;

        template <typename Func>
        typename util::function_traits<Func(future<T> &&)>::return_type then(Func && func);

        friend class promise<T>;

        template <typename Func>
        friend typename std::result_of<Func()>::type run_now(Func && func);

        // Make all other futures friend. We are a big, happy family!
        template <typename R>
        friend class IOService::future;

        friend struct detail::future_shared_state<T>;

        template <typename C>
        friend class IOService::detail::then_functor;

    protected:
    private:
        future(std::shared_ptr<detail::future_shared_state<T>> const & shared_state);

        std::shared_ptr<detail::future_shared_state<T>> _shared_state;
    };

    /*
     * future class for void type
     */
    template <>
    class future<void>
    {
    public:
        using value_type = void ;

        future();
        future(future && other) = default;
        future(future const & other) = delete;
        future & operator=(future && other) = default;
        future & operator=(future const & other) = delete;
        ~future() = default;

        void get();
        bool valid() const;

        template <typename Func>
        typename util::function_traits<Func(future<void> &&)>::return_type then(Func && func);

        friend class promise<void>;

        template <typename Func>
        friend typename std::result_of<Func()>::type run_now(Func && func);

        // Make all other futures friend. We are a big, happy family!
        template <typename R>
        friend class IOService::future;

        friend struct detail::future_shared_state<void>;

        template <typename C>
        friend class IOService::detail::then_functor;

    protected:
    private:
        future(std::shared_ptr<detail::future_shared_state<void>> const & shared_state);

        std::shared_ptr<detail::future_shared_state<void>> _shared_state;
    };


    template <typename T>
    template <typename Func>
    typename util::function_traits<Func(future<T> &&)>::return_type future<T>::then(Func && func)
    {
        using Return_Type = typename util::function_traits<Func(future<T>)>::return_type;
        auto functor = detail::then_functor<is_future<Return_Type>::value, Return_Type>();
        auto copyable_func = util::make_copyable_function<Func>(func);
        return functor(copyable_func, *this);
    }

    template <typename Func>
    typename util::function_traits<Func(future<void> &&)>::return_type future<void>::then(Func && func)
    {
        using Return_Type = typename util::function_traits<Func(future<void>)>::return_type;
        auto functor = detail::then_functor<is_future<Return_Type>::value, Return_Type>();
        auto copyable_func = util::make_copyable_function<Func>(func);
        return functor(copyable_func, *this);
    }


    template <typename T>
    future<T>::future():
        _shared_state(new detail::future_shared_state<T>)
    {
    }



    template <typename T>
    IOService::future<T>::future(std::shared_ptr<detail::future_shared_state<T>> const & shared_state)
    {
        _shared_state = shared_state;
    }



    template <typename T>
    T IOService::future<T>::get()
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

        return _shared_state->_value;
    }

    template <typename T>
    bool IOService::future<T>::valid() const
    {
        return _shared_state->_valid;
    }

    /*
     * promise class for non-void type
     */
    template <typename T>
    class promise
    {
    public:
        promise() = default;
        promise(promise && other) = default;
        promise(promise const & other) = delete;

        ~promise() = default;

        promise & operator=(promise && other) = default;
        promise & operator=(promise const & rhs ) = delete;

        IOService::future<T> get_future();

        void set_value(T const & value);
        void set_value(T && value);
        void set_value(T & value);

        void set_exception(std::exception_ptr p);

    protected:
    private:
        IOService::future<T> _future;
    };


    /*
     * promise class for void type
     */
    template <>
    class promise<void>
    {
    public:
        promise() = default;
        promise(promise && other) = default;
        promise(promise const & other) = delete;

        ~promise() = default;

        promise & operator=(promise && other) = default;
        promise & operator=(promise const & rhs ) = delete;

        IOService::future<void> get_future();

        void set_value();

        void set_exception(std::exception_ptr p);

    protected:
    private:
        IOService::future<void> _future;
    };


    template <typename T>
    IOService::future<T> promise<T>::get_future()
    {
        if (2 == _future._shared_state.use_count())
        {
            throw std::future_error(std::make_error_code(std::future_errc::future_already_retrieved));
        }
        IOService::future<T> fut(_future._shared_state);
        return std::move(fut);
    }

    template <typename T>
    void promise<T>::set_value(T const & value)
    {
        _future._shared_state->set_value(value);
    }
    template <typename T>
    void promise<T>::set_value(T && value)
    {
        _future._shared_state->set_value(std::move(value));
    }
    template <typename T>
    void promise<T>::set_value(T & value)
    {
        _future._shared_state->set_value(value);
    }

    template <typename T>
    void promise<T>::set_exception(std::exception_ptr p)
    {
        _future._shared_state->set_exception(p);
    }

    // Helper functions

    /**
     * Syntax sugar.
     * run_now() wraps the func(), invokes it, catches possible exception
     * and puts into the future<T>.
     */
    template <typename Func>
    typename std::result_of<Func()>::type run_now(Func && func)
    {
        using Value_Type = typename std::result_of<Func()>::type::value_type;
        future<Value_Type> call_future;
        try
        {
            call_future = std::move(func());
        }
        catch (...)
        {
            call_future._shared_state->set_exception(std::current_exception());
        }
        return call_future;
    }

    /**
     * Syntax sugar.
     * extract_exception() tries to convert ex_ptr to the given T exception type.
     * If it fails, the original exception will be rethrown.
     */
    template <typename T>
    T extract_exception(std::exception_ptr const & ex_ptr)
    {
        try
        {
            std::rethrow_exception(ex_ptr);
        }
        catch (T const & t)
        {
            return t;
        }
    }


    /**
     * Syntax sugar
     * pack_exception converts the given T-type exception to std::exception_ptr.
     */
    template <typename T>
    std::exception_ptr pack_exception(T && ex)
    {
        try
        {
            throw ex;
        }
        catch (...)
        {
            return std::current_exception();
        }
        return std::exception_ptr(); // Never reach this point.
    }



    template<typename T>
    struct mutable_promise
    {
        mutable_promise() = default;
        mutable_promise(mutable_promise<T> const & rhs)
        {
            promise = std::move(rhs.promise);
        }
        mutable_promise & operator=(mutable_promise<T> const & rhs)
        {
            if (&rhs != this)
            {
                promise = std::move(rhs.promise);
            }
            return *this;
        }

        IOService::future<T> get_future() const
        {
            return promise.get_future();
        }

        template <typename... Ts>
        void set_value(Ts &&... ts) const
        {
            promise.set_value(std::forward<Ts...>(ts)...);
        }

        template <typename... Ts>
        void set_exception(Ts &&... ts) const
        {
            promise.set_exception(std::forward<Ts...>(ts)...);
        }

        mutable IOService::promise<T> promise;
    };

    template<typename T>
    future<std::vector<T>> collect_all(std::vector<future<T>> & v)
    {
        std::shared_ptr<promise<std::vector<T>>> prom {new promise<std::vector<T>>()};
        std::shared_ptr<int> counter {new int(v.size())};
        std::shared_ptr<std::vector<T>> res {new std::vector<T>()};
        std::shared_ptr<std::vector<std::exception_ptr>> ex_ptrs {new std::vector<std::exception_ptr>()};

        for(auto & f : v)
        {
            f.then([prom, counter, res, ex_ptrs](future<T> result){
                //todo mutex
                (*counter)--;
                try
                {
                    res->push_back(result.get());
                }
                catch (...)
                {
                    ex_ptrs->push_back(std::current_exception());
                }
                if(0 >= *counter)
                {
                    if(ex_ptrs->size() == 0)
                        prom->set_value(*res);
                    else
                        prom->set_exception(ex_ptrs->at(0));
                }
            });
        }
        return prom->get_future();
    }

}
#endif