/*
 * semaforos.c
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#include "semaforos.h"
#include "configuracion.h"

void inicializar_semaforos() {
	lista_de_semaforos = list_create();
	semaforo_t* semaforo = malloc(sizeof(semaforo_t));
	semaforo->id = string_new();
	int i = 0;

	while (SEM_IDS[i] != NULL) {
		semaforo->id = SEM_IDS[i];
		semaforo->valor = atoi(SEM_INIT[i]);
		semaforo->valor_maximo = atoi(SEM_MAX[i]);
		list_add(lista_de_semaforos, semaforo);
		i++;
		printf("%s = %i\n", semaforo->id, semaforo->valor);
	}
}

semaforo_t* obtener_semaforo(char* nombre_semaforo) {

	bool es_semaforo_buscado(semaforo_t* semaforo) {
		return strcmp(nombre_semaforo, semaforo->id);
	}

	return list_find(lista_de_semaforos, (void*)es_semaforo_buscado);
}

int semaforo_wait(char* nombre_semaforo) {
	semaforo_t* semaforo = malloc(sizeof(semaforo_t));
	semaforo = obtener_semaforo(nombre_semaforo);

	if (semaforo->valor > 0)
		semaforo->valor--;

	printf("wait (%s = %i)\n", semaforo->id, semaforo->valor);
	return 0;
}

int semaforo_signal(char* nombre_semaforo) {
	semaforo_t* semaforo = malloc(sizeof(semaforo_t));
	semaforo = obtener_semaforo(nombre_semaforo);

	if (semaforo->valor < semaforo->valor_maximo)
		semaforo->valor++;

	printf("signal (%s = %i)\n", semaforo->id, semaforo->valor);
	return semaforo->valor;
}















