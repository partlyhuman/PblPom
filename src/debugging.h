#pragma once
#include <pebble.h>

// RELEASE is set by the build system (waf configure --debug).
// On debug builds, it simply omits this define.
// Use DEBUG as a convenience, but we have to set it here.
#ifdef RELEASE
#undef DEBUG
#define LOG(...)
#define WARN(...)
#define ERROR(...)

#else
#define DEBUG 1
#define USE_CONSOLE 1
#define LOG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
#endif


#if USE_CONSOLE
#define __CONSOLE_INCLUDE_LINENO 0
#define __CONSOLE_BUFFER_SIZE 250
#define __CONSOLE_LINE_SIZE 64
extern char __console_line[];
extern char __console_buffer[];
extern TextLayer __console_layer;


#define CONSOLE(...) \
    if (strlen(__console_buffer) > __CONSOLE_BUFFER_SIZE - __CONSOLE_LINE_SIZE - (__CONSOLE_INCLUDE_LINENO)*__CONSOLE_LINE_SIZE) __console_buffer[0] = '\0'; \
    if (__CONSOLE_INCLUDE_LINENO) snprintf(__console_line, __CONSOLE_LINE_SIZE, "%s(%d): ", __FILE__, __LINE__); \
    if (__CONSOLE_INCLUDE_LINENO) strncat(__console_buffer, __console_line, __CONSOLE_BUFFER_SIZE-strlen(__console_buffer)-1); \
    snprintf(__console_line, __CONSOLE_LINE_SIZE, __VA_ARGS__); \
    strncat(__console_buffer, __console_line, __CONSOLE_BUFFER_SIZE-strlen(__console_buffer)-1); \
    int len = strlen(__console_buffer); \
    __console_buffer[len+1] = '\0'; \
    __console_buffer[len] = '\n'; \
    text_layer_set_text(&__console_layer, __console_buffer);

#else
#define CONSOLE LOG
#endif