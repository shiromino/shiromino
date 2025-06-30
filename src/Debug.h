#pragma once
#include <cerrno>
#include <exception>
#include "SDL_log.h"
#include <stdexcept>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

// Only handle C++20 and newer
#define log_err(M, ...) \
    SDL_LogMessage(     \
        SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno() __VA_OPT__(, ) __VA_ARGS__)

#define log_warn(M, ...) \
    SDL_LogMessage(      \
        SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_WARN, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno() __VA_OPT__(, ) __VA_ARGS__)

#define log_info(M, ...) \
    SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__ __VA_OPT__(, ) __VA_ARGS__)

#define check(A, M, ...)                            \
    do                                              \
    {                                               \
        if(!(A))                                    \
        {                                           \
            log_err(M __VA_OPT__(, ) __VA_ARGS__);  \
            errno = 0;                              \
            throw std::logic_error("check failed"); \
        }                                           \
    } while(false)

#define sentinel(M, ...)                       \
    do                                         \
    {                                          \
        log_err(M __VA_OPT__(, ) __VA_ARGS__); \
        errno = 0;                             \
    } while(false)

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...)                   \
    do                                           \
    {                                            \
        if(!(A))                                 \
        {                                        \
            debug(M __VA_OPT__(, ) __VA_ARGS__); \
            errno = 0;                           \
        }                                        \
    } while(false)
