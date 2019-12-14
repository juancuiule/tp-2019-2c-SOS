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
#include <pthread.h>

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
t_list* cola_blocked;
t_list* cola_exit;

sem_t* sem_atender_clientes;
sem_t* sem_metricas;
sem_t* sem_lista_semaforos;

pthread_mutex_t mutex_multiprogramacion;
pthread_mutex_t mutex_cola_new;
pthread_mutex_t mutex_cola_blocked;
pthread_mutex_t mutex_cola_exit;
pthread_mutex_t mutex_log;
pthread_mutex_t mutex_semaforos;

t_list* semaforos;

#endif /* GLOBALES_H_ */
