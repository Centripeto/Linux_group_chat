#include "logger.h"

#define LOG_FILE "conversation_log.txt"

extern int errno;

void eventLogger(const char* message, ...) {
  va_list args;
  va_start(args, message);
  
  time_t now = time(NULL);
  struct tm *timeinfo = localtime(&now);
  
  FILE* log_file = fopen(LOG_FILE, "a");
  if(log_file == NULL) {
    printf("An error occurred while trying to open the log file.\n");
    return; 
  }
  
  fprintf(log_file, "[%02d-%02d-%d - %02d:%02d:%02d] ", 
    timeinfo->tm_mday,
    timeinfo->tm_mon + 1,
    timeinfo->tm_year + 1900,
    timeinfo->tm_hour, 
    timeinfo->tm_min,
    timeinfo->tm_sec);

  vfprintf(log_file, message, args);
  if(errno != 0) {
    fprintf(log_file, " Exit with code: %d.", errno);
  }
  
  fprintf(log_file, "\n");

  fclose(log_file);
  va_end(args);
}
