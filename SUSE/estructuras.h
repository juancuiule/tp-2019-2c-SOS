/*
 * estructuras.h
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#ifndef ESTRUCTURAS_H_
#define ESTRUCTURAS_H_

#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <sys/time.h>

typedef struct {
	int tid;
	int pid;
	long tiempo_espera;
	long tiempo_cpu;
	long tiempo_creacion;
	long tiempo_ultima_llegada_a_ready;
	long tiempo_ultima_llegada_a_exec;
	long long estimacion_anterior;
	long long rafaga_anterior;
	int tid_hilo_esperando;
} __attribute__((packed)) hilo_t;

typedef struct {
	int pid;
	t_list* hilos_en_ready;
	hilo_t* hilo_en_exec;
} programa_t;

typedef enum {
	INIT,
	CREATE,
	SCHEDULE_NEXT,
	JOIN,
	CLOSE,
	WAIT,
	SIGNAL
} operacion;

typedef struct {
	int tid;
	int tamanio_id;
	char* id_semaforo;
} wait_t;

typedef struct {
	int tid;
	int tamanio_id;
	char* id_semaforo;
} signal_t;











#endif /* ESTRUCTURAS_H_ */
