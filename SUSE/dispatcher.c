/*
 * dispatcher.c
 *
 *  Created on: 26 oct. 2019
 *      Author: utnso
 */

#include "dispatcher.h"

void inicializar_dispatcher() {
	cola_ready = queue_create();
	cola_exec = queue_create();
}

void dispatcher(int tid) {
	inicializar_dispatcher();
	printf("SUSE: nuevo dispatcher creado (TID %i)\n", tid);
}
