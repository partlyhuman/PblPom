#pragma once
#include "pebble_os.h"

void strclear(const char*);

// RELEASE is set by the build script. On debug builds, it simply omits this define.
#ifdef RELEASE

#undef DEBUG
#define LOG(...)
#define WARN(...)
#define ERROR(...)
#define CONSOLE(...)

#else //---------------------------------------------------------------------------

#define DEBUG 1
#define LOG(...) app_log(APP_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
#define WARN(...) app_log(APP_LOG_LEVEL_WARNING, __FILE__, __LINE__, __VA_ARGS__)
#define ERROR(...) app_log(APP_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)

#define __CONSOLE_INCLUDE_LINENO 0
#define __CONSOLE_BUFFER_SIZE 250
#define __CONSOLE_LINE_SIZE 64
extern char __console_line[];
extern char __console_buffer[];
extern TextLayer __console_layer;

#define CONSOLE(...) \
    if (__CONSOLE_INCLUDE_LINENO) snprintf(__console_line, __CONSOLE_LINE_SIZE, "%s(%d): ", __FILE__, __LINE__); \
    if (__CONSOLE_INCLUDE_LINENO) strncat(__console_buffer, __console_line, __CONSOLE_BUFFER_SIZE-strlen(__console_buffer)-1); \
    snprintf(__console_line, __CONSOLE_LINE_SIZE, __VA_ARGS__); \
    strncat(__console_buffer, __console_line, __CONSOLE_BUFFER_SIZE-strlen(__console_buffer)-1); \
    int len = strlen(__console_buffer); \
    __console_buffer[len+1] = '\0'; \
    __console_buffer[len] = '\n'; \
    text_layer_set_text(&__console_layer, __console_buffer);

#define __CONSOLE_INIT \
    text_layer_init(&__console_layer, GRect(0, 28, 144, 140)); \
    text_layer_set_overflow_mode(&__console_layer, GTextOverflowModeWordWrap); \
    text_layer_set_font(&__console_layer, fonts_get_system_font(FONT_KEY_FONT_FALLBACK));

#endif