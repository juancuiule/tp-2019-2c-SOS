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

unsigned long long tiempo_actual() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long tiempo = (((unsigned long long )tv.tv_sec) * 1000 + ((unsigned long) tv.tv_usec) / 1000);
	return tiempo;
}

int tiempo_de_ejecucion(hilo_t* hilo) {
	unsigned long long actual = tiempo_actual();
	int tiempo_ejecucion =  actual - hilo->tiempo_creacion;
	return tiempo_ejecucion;
}

void logear_metricas_hilo(hilo_t* hilo) {
	if (hilo != NULL) {
		log_info(logger_metricas, "Métricas del hilo %i del programa %i: ", hilo->tid, hilo->pid);
		int tiempo_ejecucion = tiempo_de_ejecucion(hilo);
		log_info(logger_metricas, "\tTiempo de ejecución: %i ms", tiempo_ejecucion);
		log_info(logger_metricas, "\tTiempo de espera: %llu ms", hilo->tiempo_espera);
		log_info(logger_metricas, "\tTiempo de uso de CPU: %llu ms", hilo->tiempo_cpu);
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

int cantidad_de_hilos_en_new_de_programa(programa_t* programa) {

	bool es_de_programa(hilo_t* hilo) {
		return hilo->pid == programa->pid;
	}

	return list_count_satisfying(cola_new->elements, es_de_programa);
}

int cantidad_de_hilos_en_ready_de_programa(programa_t* programa) {
	return list_size(programa->hilos_en_ready);
}

int cantidad_de_hilos_en_exec_de_programa(programa_t* programa) {
	if (programa->hilo_en_exec == NULL)
		return 0;
	else
		return 1;
}

int cantidad_de_hilos_en_blocked_de_programa(programa_t* programa) {
	bool es_de_programa(hilo_t* hilo) {
		return hilo->pid == programa->pid;
	}

	return list_count_satisfying(cola_blocked, es_de_programa);
}

void logear_estados_programa(programa_t* programa) {
	log_info(logger, "Programa %i:", programa->pid);
	log_info(logger, "\tHilos en NEW: %i", cantidad_de_hilos_en_new_de_programa(programa));
	log_info(logger, "\tHilos en READY: %i", cantidad_de_hilos_en_ready_de_programa(programa));
	log_info(logger, "\tHilos en EXEC: %i", cantidad_de_hilos_en_exec_de_programa(programa));
	log_info(logger, "\tHilos en BLOCKED: %i", cantidad_de_hilos_en_blocked_de_programa(programa));
}

void logear_metricas() {
	log_info(logger, "Métricas del sistema:");
	log_info(logger, "Grado de multiprogramación: %i", GRADO_MULTIPROGRAMACION);
	dictionary_iterator(diccionario_semaforos, logear_valor_semaforo);
	list_iterate(programas, logear_estados_programa);
	list_iterate(cola_new->elements, (void*)logear_metricas_hilo);
	list_iterate(cola_blocked, (void*)logear_metricas_hilo);
	list_iterate(programas, (void*)logear_metricas_hilos_programa);
}

void logear_metricas_timer() {

	while (1) {
		logear_metricas();
		sleep(METRICS_TIMER);
	}
}
