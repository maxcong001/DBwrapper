#pragma once
#include "util.hpp"



// interface form APP API to redis RSP message
template <typename T>
class redis_command
{
  public:
    using key_type = std::remove_const_t<std::remove_reference_t<T.key>>;
    using value_type = std::remove_const_t<std::remove_reference_t<T.value>>;
    redis_command() = default;

    static std::string get_command(T &&val)
    {
        switch (val.type)
        {
        case MSG_TYPE::TASK_REDIS_PUT:
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
            break;
        case MSG_TYPE::TASK_REDIS_GET:
            break;
        case MSG_TYPE::TASK_REDIS_DEL:
            break;
        case MSG_TYPE::TASK_ADD_CONN:
            break;
        case MSG_TYPE::TASK_DEL_CONN:
            break;
        case MSG_TYPE::TASK_REDIS_PING:
            break;
        default:
            __LOG(warn, "unsupport message type!");
            break;
        }
    }

};