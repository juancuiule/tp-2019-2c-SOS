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
#include <commons/string.h>

typedef struct {
	int tid;
	int pid;
	int tiempo_ejecucion;
	int tiempo_espera;
	int tiempo_cpu;
	float estimacion_anterior;
	float rafaga_anterior;
	t_queue* hilos_esperando;
} __attribute__((packed)) hilo_t;

typedef struct {
	int pid;
	t_queue* cola_ready;
	hilo_t* hilo_en_exec;
} programa_t;

typedef enum {
	CREATE,
	SCHEDULE_NEXT,
	JOIN,
	CLOSE,
	WAIT,
	SIGNAL
} operacion;

typedef struct {
	char* id;
	int valor;
} semaforo_t;

t_dictionary* diccionario_programas;
t_dictionary* diccionario_tid_pid;
t_dictionary* diccionario_tid;

void* serializar(hilo_t);
int ejecutar_operacion(int, int);

#endif /* LIBSUSE_H_ */
