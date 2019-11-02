/*
 * server.h
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#ifndef SERVIDOR_H_
#define SERVIDOR_H_

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
#include "configuracion.h"

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;

pthread_t nuevo_ult;

void inicializar_colas();
int servidor();

#endif /* SERVIDOR_H_ */
