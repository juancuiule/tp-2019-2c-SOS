/*
 * semaforos.h
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <stdio.h>
#include <commons/collections/dictionary.h>

typedef struct {
	int valor;
	int valor_maximo;
} sem_value_t;

t_dictionary* diccionario_semaforos;

void inicializar_diccionario_semaforos();
int semaforo_wait(char* nombre_semaforo);
int semaforo_signal(char* nombre_semaforo);
sem_value_t* obtener_semaforo(char*);

#endif /* SEMAFOROS_H_ */
