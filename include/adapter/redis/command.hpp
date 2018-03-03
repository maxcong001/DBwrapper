#pragma once
#include "util.hpp"

// interface form APP API to redis RSP message
template <typename COMMAND_KEY, typename COMMAND_VALUE, typename COMMAND_ARGS = std::nullptr_t> // to do typename... COMMAND_ARGS>
class redis_command
{
    using List = std::list<std::string>;

  public:
#if __cplusplus >= 201703L
    using key_type = std::remove_const_t<std::remove_reference_t<COMMAND_KEY>>;
    using value_type = std::remove_const_t<std::remove_reference_t<COMMAND_VALUE>>;
#endif
    redis_command() = default;

    static std::string get_format_command(MSG_TYPE type, COMMAND_KEY key, COMMAND_VALUE value, COMMAND_ARGS args = nullptr) // to do COMMAND_ARGS... args)
    {
        switch (type)
        {
        case MSG_TYPE::TASK_REDIS_PUT:
#if __cplusplus >= 201703L
            if constexpr (detail::is_string_v<COMMAND_KEY>)
            {
                if constexpr (detail::is_string_v<COMMAND_VALUE>)
                {
                    __LOG(error, "Put command");
                    List _list;
                    _list.emplace_back("SET");
                    _list.emplace_back(key);
                    _list.emplace_back(value);
                    return redis_formatCommand(_list);
                }
                else
                {
                }
            }
            else
            {
            }
#else
            if (std::is_same<COMMAND_KEY, std::string>::value)
            {
                if (std::is_same<COMMAND_VALUE, std::string>::value)
                {
                    __LOG(error, "Put command");
                    List _list;
                    _list.emplace_back("SET");
                    _list.emplace_back(key);
                    _list.emplace_back(value);
                    return redis_formatCommand(_list);
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

    static std::string redis_formatCommand(List &argv)
    {
        __LOG(warn, "[redis_formatCommand]");
        std::ostringstream buffer;
        buffer << "*" << argv.size() << "\r\n";
        List::const_iterator iter = argv.begin();
        while (iter != argv.end())
        {
            buffer << "$" << iter->size() << "\r\n";
            buffer << *iter++ << "\r\n";
        }
        return buffer.str();
    }
};