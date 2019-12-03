/*
 * globales.h
 *
 *  Created on: 1 dic. 2019
 *      Author: utnso
 */

#ifndef GLOBALES_H_
#define GLOBALES_H_

#include <semaphore.h>
#include <commons/log.h>
#include <commons/collections/queue.h>
#include <commons/collections/list.h>
#include "semaforos.h"

int servidor_fd;
int tid_hilo_buscado;
int pid_programa_buscado;
int tid_hilo_anterior;
int tid_siguiente_hilo;

int PID;
int TID;
int GRADO_MULTIPROGRAMACION;

t_log* logger;
t_log* logger_metricas;

t_list* programas;
t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;

sem_t* tid_sem;
sem_t* pid_sem;
sem_t* multiprogramacion_sem;

sem_value_t* sem_value;

#endif /* GLOBALES_H_ */
