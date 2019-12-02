/*
 * semaforos.h
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <stdio.h>

typedef struct {
	char* id;
	int valor;
} semaforo_t;

semaforo_t* semaforos;

void inicializar_semaforos(semaforo_t*);
void semaforo_wait(semaforo_t*);
void semaforo_signal(semaforo_t*);

#endif /* SEMAFOROS_H_ */
