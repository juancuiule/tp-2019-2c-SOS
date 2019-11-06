/*
 * configuracion.c
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#include "configuracion.h"

void configurar() {
	config = config_create("../SUSE.config");
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	diccionario_procesos = dictionary_create();
	diccionario_ults = dictionary_create();

	LISTEN_IP = config_get_string_value(config, "LISTEN_IP");
	LISTEN_PORT = config_get_string_value(config, "LISTEN_PORT");
	METRICS_TIMER = config_get_int_value(config, "METRICS_TIMER");
	MAX_MULTIPROG = config_get_int_value(config, "MAX_MULTIPROG");
	SEM_IDS = config_get_array_value(config, "SEM_IDS");
	SEM_INIT = config_get_array_value(config, "SEM_INIT");
	SEM_MAX = config_get_array_value(config, "SEM_MAX");
	ALPHA_SJF = config_get_double_value(config, "ALPHA_SJF");
}
