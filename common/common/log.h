#ifndef COMMON_LOG_H_
#define COMMON_LOG_H_

#define LOG_ENABLED true

#include <commons/string.h>
#include <commons/log.h>
#include <stdbool.h>

#define BUFFER_SIZE 256

void log_iniciar(char* file, char *name, bool is_active_console);
void log_msje_info(const char *format, ...);
void log_msje_error(const char *format, ...);
void log_liberar();

#endif
