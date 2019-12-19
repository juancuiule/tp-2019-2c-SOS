#include "log.h"

static t_log *logger = NULL;

void log_iniciar(char* file, char *name, bool is_active_console)
{
	if(logger == NULL)
	{
		logger = log_create(
				file,
				name,
				is_active_console,
				LOG_LEVEL_TRACE);
	}

}

static void log_template(bool error, const char *format, va_list args)
{
	if(!LOG_ENABLED) return;

	if(logger == NULL)
	{
		log_iniciar("log.txt", "log", true);
	}

	char mensaje[BUFFER_SIZE];

	vsnprintf(mensaje, BUFFER_SIZE, format, args);

	if(error)
	{
		log_error(logger, mensaje);
	} else
	{
		log_debug(logger, mensaje);
	}
}

void log_msje_info(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_template(false, format, args);
	va_end(args);
}

void log_msje_error(const char *format, ...)
{
	va_list args;
	va_start(args, format);
	log_template(true, format, args);
	va_end(args);
}

void log_liberar()
{
	log_destroy(logger);
}
