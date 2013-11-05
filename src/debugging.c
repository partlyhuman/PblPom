#include "debugging.h"

#if USE_CONSOLE
char __console_line[__CONSOLE_LINE_SIZE] = {'\0'};
char __console_buffer[__CONSOLE_BUFFER_SIZE] = {'\0'};
TextLayer __console_layer;
#endif