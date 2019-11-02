/*
 * dispatcher.c
 *
 *  Created on: 26 oct. 2019
 *      Author: utnso
 */

#include "dispatcher.h"

void inicializar_colas_d() {
	cola_ready = queue_create();
	cola_exec = queue_create();
}

void dispatcher(int tid) {
	inicializar_colas();
	printf("Ha ingresado al planificador el ULT con el TID %i\n", tid);
}
