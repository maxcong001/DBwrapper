#pragma once
#include "task_base/include.hpp"
#include <type_traits>
#include <cstddef>
#include <adapter/redis/hiredis/hiredis.h>
#include <adapter/redis/hiredis/async.h>
#include <adapter/redis/hiredis/adapters/libevent.h>
#define WORKER001 "WORKER001"
struct TASK_REDIS_RAW_MSG
{
    void * usr_data;
    redisCallbackFn *cb;
    std::string body;
};
struct TASK_REDIS_PUT_MSG
{
    void * usr_data;
    redisCallbackFn *cb;
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
namespace detail
{
#if __cplusplus >= 201703L
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
#endif
}

typedef std::function<void(void)> redis_user_cb;

struct command_container
{
    MSG_TYPE type;
    void *usr_data;
    redis_user_cb cb;
};

struct put_command_container
{
    MSG_TYPE type;
    void *usr_data;
    redis_user_cb cb;
    std::string key;
    std::string msg_value;
};

enum class conn_status : std::uint32_t
{
    CONNECTED = 0,
    CONNECTING,
    DISCONNECTED
};
//  task payload definition
struct add_conn_payload
{
    std::string ip;
    int port;
};