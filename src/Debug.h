/**
 * Copyright (c) 2020 Brandon McGriff
 *
 * Licensed under the MIT license; see the LICENSE file at the top level
 * directory for the full text of the license.
 */
// TODO: Go over the code, changing uses of "goto error;" to handle true/false
// values of check* macros.
#pragma once
#include <cerrno>
#include <exception>
#include <SDL_log.h>
#include <stdexcept>

#ifdef NDEBUG
#define debug(M, ...)
#else
#define debug(M, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG, "[DEBUG] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#define clean_errno() (errno == 0 ? "None" : strerror(errno))

#define log_err(M, ...) SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_ERROR, "[ERROR] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_warn(M, ...) SDL_LogMessage(SDL_LOG_CATEGORY_ERROR, SDL_LOG_PRIORITY_WARN, "[WARN] (%s:%d: errno: %s) " M "\n", __FILE__, __LINE__, clean_errno(), ##__VA_ARGS__)

#define log_info(M, ...) SDL_LogMessage(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#define check(A, M, ...) do { if (!(A)) { log_err(M, ##__VA_ARGS__); errno = 0; throw std::logic_error("check failed"); } } while (false)

#define sentinel(M, ...) do { log_err(M, ##__VA_ARGS__); errno = 0; } while (false)

#define check_mem(A) check((A), "Out of memory.")

#define check_debug(A, M, ...) do { if (!(A)) { debug(M, ##__VA_ARGS__); errno = 0; } while (false)