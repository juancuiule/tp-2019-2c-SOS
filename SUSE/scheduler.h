/*
 * server.h
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <commons/collections/queue.h>
#include <pthread.h>

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;

void inicializar_colas();
int scheduler();

#endif /* SCHEDULER_H_ */
