#include "debugging.h"

#if USE_CONSOLE
char __console_line[__CONSOLE_LINE_SIZE];
char __console_buffer[__CONSOLE_BUFFER_SIZE];
TextLayer __console_layer;
#endif