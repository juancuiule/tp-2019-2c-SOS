/*
 * semaforos.h
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include <stdio.h>
#include <commons/collections/list.h>

typedef struct {
	char* id;
	int valor;
	int valor_maximo;
} semaforo_t;

t_list* lista_de_semaforos;

void inicializar_semaforos();
void imprimir_semaforos();
int semaforo_wait(char* nombre_semaforo);
int semaforo_signal(char* nombre_semaforo);
semaforo_t* obtener_semaforo(char*);

#endif /* SEMAFOROS_H_ */
