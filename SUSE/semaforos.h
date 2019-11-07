/*
 * semaforos.h
 *
 *  Created on: 6 nov. 2019
 *      Author: utnso
 */

#ifndef SEMAFOROS_H_
#define SEMAFOROS_H_

#include "configuracion.h"
#include <libSUSE/libSUSE.h>

semaforo_t** semaforos;

void inicializar_semaforos(semaforo_t**);
void semaforo_wait(semaforo_t*);
void semaforo_signal(semaforo_t*);

#endif /* SEMAFOROS_H_ */
