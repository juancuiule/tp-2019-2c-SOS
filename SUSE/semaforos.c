/*
 * semaforos.c
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#include "semaforos.h"
#include "configuracion.h"

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

int semaforo_wait(semaforo_t* semaforo) {
	if (semaforo->valor > 0)
		semaforo->valor--;

	printf("recibi un wait (semáforo = 0)\n");
	return semaforo->valor;
}

int semaforo_signal(semaforo_t* semaforo) {
	semaforo->valor++;
	printf("recibi un signal (semáforo = %i)\n", semaforo->valor);
	return semaforo->valor;
}

