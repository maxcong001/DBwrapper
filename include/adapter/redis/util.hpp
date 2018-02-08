#pragma once
#include "task_base/include.hpp"
#define WORKER001 "WORKER001"
struct TASK_REDIS_PUT_MSG
{
    std::string body;
};
struct TASK_REDIS_GET_MSG
{
};
struct TASK_REDIS_DEL_MSG
{
};
struct TASK_REDIS_PING_MSG
{
};
struct TASK_ADD_CONN_MSG
{
};
struct TASK_DEL_CONN_MSG
{
};

#if !__cplusplus >= 201703L
namespace std
{
template <class T, class U>
inline constexpr bool is_same_v = is_same<T, U>::value;
}
#endif

namespace detail
{

template <typename T>
constexpr bool is_int64_v = std::is_same_v<T, std::int64_t>;

template <typename T>
constexpr bool is_uint64_v = std::is_same_v<T, std::uint64_t>;

template <typename T>
constexpr bool is_64_v = std::is_same_v<T, std::int64_t> || std::is_same_v<T, std::uint64_t>;

template <typename T>
constexpr bool is_string_v = std::is_same_v<T, std::string>;

template <typename T>
constexpr bool is_cstr_v = std::is_same_v<T, const char *>;
}

typedef std::function<void(void)> redis_user_cb;
struct command_container
{
    MSG_TYPE type;
    void *usr_data;
    redis_user_cb cb;
};
struct put_command_container : public command_container
{
    std::string key;
    std::string value;
}