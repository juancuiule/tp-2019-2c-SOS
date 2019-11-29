/*
 * logger.c
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#include "logger.h"

void inicializar_loggers() {
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);
}
