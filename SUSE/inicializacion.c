/*
 * inicializacion.c
 *
 *  Created on: 1 dic. 2019
 *      Author: utnso
 */

#include "inicializacion.h"
#include "globales.h"
#include "configuracion.h"
#include "estructuras.h"

void inicializar_semaforos() {
	semaforos = list_create();
	int i = 0;

	while (SEM_IDS[i] != NULL) {
		semaforo_t* semaforo = malloc(sizeof(semaforo_t));
		semaforo->id = SEM_IDS[i];
		semaforo->valor_actual = atoi(SEM_INIT[i]);
		semaforo->valor_maximo = atoi(SEM_MAX[i]);
		semaforo->hilos_bloqueados = list_create();
		list_add(semaforos, semaforo);
		i++;
	}
}

void inicializar() {
	PID = 0;
	TID = 0;
	GRADO_MULTIPROGRAMACION = 0;

	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);

	programas = list_create();
	cola_new = queue_create();
	cola_blocked = list_create();
	cola_exit = list_create();

	sem_init(&sem_atender_clientes, 0, 1);
	sem_init(&sem_metricas, 0, 1);

	pthread_mutex_init(&mutex_multiprogramacion, NULL);

	inicializar_semaforos();
}
