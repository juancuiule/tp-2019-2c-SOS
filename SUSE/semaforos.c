/*
 * semaforos.c
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#include "semaforos.h"
#include "configuracion.h"
#include "globales.h"

void inicializar_diccionario_semaforos() {
	diccionario_semaforos = dictionary_create();
	sem_value = malloc(sizeof(sem_value_t));
	int i = 0;

	while (SEM_IDS[i] != NULL) {
		sem_value->valor = atoi(SEM_INIT[i]);
		sem_value->valor_maximo = atoi(SEM_MAX[i]);
		dictionary_put(diccionario_semaforos, SEM_IDS[i], sem_value);
		sem_value = malloc(sizeof(sem_value_t));
		i++;
	}
}

int semaforo_wait(char* nombre_semaforo) {
	sem_value_t* sem_value = malloc(sizeof(sem_value_t));
	sem_value = dictionary_get(diccionario_semaforos, nombre_semaforo);

	if (sem_value->valor > 0)
		sem_value->valor--;

	//printf("%s = %i\n", nombre_semaforo, sem_value->valor);
	dictionary_put(diccionario_semaforos, nombre_semaforo, sem_value);
	return sem_value->valor;
}

int semaforo_signal(char* nombre_semaforo) {
	sem_value_t* sem_value = malloc(sizeof(sem_value_t));
	sem_value = dictionary_get(diccionario_semaforos, nombre_semaforo);
	sem_value->valor++;
	//printf("%s = %i\n", nombre_semaforo, sem_value->valor);
	dictionary_put(diccionario_semaforos, nombre_semaforo, sem_value);
	return sem_value->valor;
}















