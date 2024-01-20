#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <stdarg.h>

#define LOG_FILE "conversation_log.txt"

void eventLogger(const char* message, ...);
