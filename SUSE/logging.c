/*
 * logging.c
 *
 *  Created on: 1 dic. 2019
 *      Author: utnso
 */

#include "logging.h"
#include "estructuras.h"
#include "globales.h"
#include "configuracion.h"

long tiempo_de_ejecucion(hilo_t* hilo) {
	return tiempo_actual() - hilo->tiempo_creacion;
}

void logear_metricas_hilo(hilo_t* hilo) {
	if (hilo != NULL) {
		log_info(logger_metricas, "Métricas del hilo %i: ", hilo->tid);
		long tiempo_ejecucion = tiempo_de_ejecucion(hilo);
		log_info(logger_metricas, "\ttiempo de ejecución: %ld ms", tiempo_ejecucion);
		log_info(logger_metricas, "\ttiempo de espera: %ld ms", hilo->tiempo_espera);
		log_info(logger_metricas, "\ttiempo de uso de CPU: %ld ms", hilo->tiempo_cpu);
	}
}

void logear_metricas_hilos_programa(programa_t* programa) {
	hilo_t* hilo_en_ejecucion = malloc(sizeof(hilo_t));
	hilo_en_ejecucion = programa->hilo_en_exec;
	t_list* hilos_programa = list_create();
	list_add(hilos_programa, hilo_en_ejecucion);
	list_add_all(hilos_programa, programa->hilos_en_ready);
	list_iterate(hilos_programa, (void*)logear_metricas_hilo);
}

void logear_valor_semaforo(char* id, sem_value_t* sem_value) {
	log_info(logger_metricas, "%s = %i", id, sem_value->valor);
}

void logear_metricas() {

	while (1) {
		sleep(METRICS_TIMER);
		log_info(logger_metricas, "Grado de multiprogramación: %i", GRADO_MULTIPROGRAMACION);
		list_iterate(cola_new->elements, (void*)logear_metricas_hilo);
		list_iterate(cola_blocked->elements, (void*)logear_metricas_hilo);
		list_iterate(programas, (void*)logear_metricas_hilos_programa);
		dictionary_iterator(diccionario_semaforos, logear_valor_semaforo);
	}
}
