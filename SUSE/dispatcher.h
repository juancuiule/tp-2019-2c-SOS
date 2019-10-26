/*
 * dispatcher.h
 *
 *  Created on: 26 oct. 2019
 *      Author: utnso
 */

#ifndef DISPATCHER_H_
#define DISPATCHER_H_

#include <stdio.h>
#include <commons/collections/queue.h>

t_queue* cola_ready;
t_queue* cola_exec;

void inicializar_colas_d();
void dispatcher();


#endif /* DISPATCHER_H_ */
