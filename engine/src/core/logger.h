#pragma once

#include "defines.h"

#define LOG_WARN_ENABLED 1
#define LOG_INFO_ENABLED 1
#define LOG_DEBUG_ENABLED 1
#define LOG_TRACE_ENABLED 1

// Disable debug and trace logging for release builds.
#if MRELEASE == 1
#define LOG_DEBUG_ENABLED 0
#define LOG_TRACE_ENABLED 0
#endif

typedef enum log_level
{
    LOG_LEVEL_FATAL = 0,
    LOG_LEVEL_ERROR = 1,
    LOG_LEVEL_WARN = 2,
    LOG_LEVEL_INFO = 3,
    LOG_LEVEL_DEBUG = 4,
    LOG_LEVEL_TRACE = 5,
} log_level;

/**
 * @brief Initialises logging system. Call twice; once with a null state to get required memory size,
 * then a second time passing allocated memory to state.
 * 
 * @param memory_requirement A pointer to hold the required memory size of internal state.
 * @param state Allocated block of memory.
 * @return True on success; otherwise false.
 */
b8 logging_system_initialise(u64 *memory_requirement, void *state);
void shutdown_logging(void *state);

MAPI void log_output(log_level level, const char *message, ...);

// Logs a fatal-level message
#define MFATAL(message, ...) log_output(LOG_LEVEL_FATAL, message, ##__VA_ARGS__);

#ifndef MERROR
// Logs an error-level message
#define MERROR(message, ...) log_output(LOG_LEVEL_ERROR, message, ##__VA_ARGS__);
#endif

#if LOG_WARN_ENABLED == 1
// Logs an warning-level message
#define MWARN(message, ...) log_output(LOG_LEVEL_WARN, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_WARN_ENABLED != 1
#define MWARN(message, ...)
#endif

#if LOG_INFO_ENABLED == 1
// Logs an info-level message
#define MINFO(message, ...) log_output(LOG_LEVEL_INFO, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_INFO_ENABLED != 1
#define MINFO(message, ...)
#endif

#if LOG_DEBUG_ENABLED == 1
// Logs an debug-level message
#define MDEBUG(message, ...) log_output(LOG_LEVEL_DEBUG, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_DEBUG_ENABLED != 1
#define MDEBUG(message, ...)
#endif

#if LOG_TRACE_ENABLED == 1
// Logs an trace-level message
#define MTRACE(message, ...) log_output(LOG_LEVEL_TRACE, message, ##__VA_ARGS__);
#else
// Does nothing when LOG_TRACE_ENABLED != 1
#define MTRACE(message, ...)
#endif
