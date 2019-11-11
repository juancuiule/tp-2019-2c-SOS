/*
 * semaforos.c
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#include "semaforos.h"

void inicializar_semaforos(semaforo_t* semaforos) {
	SEM_IDS = malloc(sizeof(char));
	int i = 0;

	while (SEM_IDS[i] != NULL) {
		semaforos[i].id = SEM_IDS[0];
		semaforos[i].valor = SEM_INIT[0];
		i++;
	}

	free(SEM_IDS);
}

void semaforo_wait(semaforo_t semaforo) {
	semaforo.valor--;

	while (semaforo.valor != 0);
}

void semaforo_signal(semaforo_t semaforo) {
	semaforo.valor++;
}
