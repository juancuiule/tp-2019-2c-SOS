/*
 * libSUSE.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef LIBSUSE_H_
#define LIBSUSE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

typedef struct {
	int tid;
	int pid;
	int tiempo_ejecucion;
	int tiempo_espera;
	int tiempo_cpu;
} hilo_t;

typedef struct {
	int pid;
	t_queue* cola_ready;
	hilo_t* exec;
} programa_t;

typedef enum {
	CREATE, // 1
	SCHEDULE_NEXT, // 2
	JOIN, // 3
	CLOSE, // 4
	WAIT, // 5
	SIGNAL // 6
} operacion_t;

typedef struct {
	int operacion;
	hilo_t* hilo;
} mensaje_t;

typedef struct {
	char* id;
	int valor;
} semaforo_t;

t_dictionary* diccionario_programas;
t_dictionary* diccionario_tid_pid;
t_dictionary* diccionario_tid;

void* serializar(hilo_t);

#endif /* LIBSUSE_H_ */
