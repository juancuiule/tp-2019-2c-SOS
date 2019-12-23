/*
 * semaforos.c
 *
 *  Created on: 12 dic. 2019
 *      Author: utnso
 */

#include <stdbool.h>
#include "semaforos.h"
#include "globales.h"

semaforo_t* obtener_semaforo(char* id_semaforo) {

	bool es_semaforo_buscado(semaforo_t* semaforo) {
		return strcmp(semaforo->id, id_semaforo) == 0;
	}

	return list_find(semaforos, es_semaforo_buscado);
}

void imprimir_hilos_esperando_semaforo(char* id_semaforo) {
	semaforo_t* semaforo = malloc(sizeof(semaforo_t));
	semaforo = obtener_semaforo(id_semaforo);

	void imprimir_hilo(hilo_t* hilo) {
		printf("\tTID: %i, PID: %i\n", hilo->tid, hilo->pid);
	}

	printf("Los hilos bloqueados esperando el semáforo %s son: \n", id_semaforo);
	list_iterate(semaforo->hilos_bloqueados, imprimir_hilo);
}
