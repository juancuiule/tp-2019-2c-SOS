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
	cola_blocked = queue_create();
	cola_exit = queue_create();

	tid_sem = malloc(sizeof(sem_t));
	sem_init(tid_sem, 0 , 1);

	pid_sem = malloc(sizeof(sem_t));
	sem_init(pid_sem, 0, 1);

	multiprogramacion_sem = malloc(sizeof(sem_t));
	sem_init(multiprogramacion_sem, 0, 1);

	sem_value = malloc(sizeof(sem_value_t));
}
