#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils.h"
#include "configuracion.h"
#include <commons/string.h>
#include <stdbool.h>

int pid_programa_buscado;

int PID = 0;
int TID = 0;
int GRADO_MULTIPROGRAMACION = 0;

t_log* logger;
t_log* logger_metricas;

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
t_list* programas;

sem_t* tid_sem;
sem_t* pid_sem;
sem_t* multiprogramacion_sem;

void inicializar();
void logear_metricas();
void atender_cliente(int);
void atender_nuevo_cliente(int);
void llega_nuevo_hilo(hilo_t*);
void encolar_hilo_en_new(hilo_t*);
void encolar_hilo_en_ready();
int programa_nuevo(hilo_t*);
void agregar_programa(hilo_t*);
int obtener_indice_de_programa(int);
hilo_t* siguiente_hilo_a_ejecutar(programa_t*);
void ejecutar_nuevo_hilo(hilo_t*);
void liberar();
