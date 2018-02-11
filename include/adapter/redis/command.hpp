#pragma once
#include "util.hpp"

// interface form APP API to redis RSP message
template <typename T>
class redis_command
{
  public:
#if __cplusplus >= 201703L
    using key_type = std::remove_const_t<std::remove_reference_t<T.key>>;
    using value_type = std::remove_const_t<std::remove_reference_t<T.value>>;
#else
    //typedef decltype(T::key) key_type;
    //typedef decltype(T::value) value_type
//#define key_type decltype(T.key)
//#define value_type decltype(T.value)
#endif
    redis_command() = default;

    static std::string get_command(T val)
    {
        switch (val.type)
        {
        case MSG_TYPE::TASK_REDIS_PUT:
#if __cplusplus >= 201703L
            if constexpr (detail::is_string_v<key_type>)
            {
                if constexpr (detail::is_string_v<value_type>)
                {
                    __LOG(debug, "");
                    return "SET " + val.key + " " + val.value;
                }
                else
                {
                }
            }
            else
            {
            }
#else
            if (std::is_same<decltype(T::key), std::string>::value)
            {
                if (std::is_same<decltype(T::msg_value), std::string>::value)
                {
                    __LOG(debug, "Put command");
                    return "SET " + val.key + " " + val.msg_value;
                }
            }
#endif
            break;
        case MSG_TYPE::TASK_REDIS_GET:
            break;
        case MSG_TYPE::TASK_REDIS_DEL:
            break;
        case MSG_TYPE::TASK_REDIS_ADD_CONN:
            break;
        case MSG_TYPE::TASK_REDIS_DEL_CONN:
            break;
        case MSG_TYPE::TASK_REDIS_PING:
            break;
        default:
            __LOG(warn, "unsupport message type!");
            break;
        }
        return "";
    }
};