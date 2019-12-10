/*
 * inicializacion.c
 *
 *  Created on: 1 dic. 2019
 *      Author: utnso
 */

#include "inicializacion.h"
#include "globales.h"

void inicializar() {
	PID = 0;
	TID = 0;
	GRADO_MULTIPROGRAMACION = 0;

	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);

	programas = list_create();
	cola_new = queue_create();
	cola_blocked = list_create();
	cola_exit = queue_create();

	sem_init(&sem_atender_clientes, 0, 1);
	sem_init(&sem_metricas, 0, 1);

	pthread_mutex_init(&mutex_multiprogramacion, NULL);

	sem_value = malloc(sizeof(sem_value_t));
}
