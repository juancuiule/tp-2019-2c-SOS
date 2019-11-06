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

int PID;

t_queue* cola_ready;
t_queue* cola_exec;

void inicializar_dispatcher();
void dispatcher();
void setear_PID();


#endif /* DISPATCHER_H_ */
