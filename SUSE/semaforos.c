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
