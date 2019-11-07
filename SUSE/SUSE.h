#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils.h"
#include "configuracion.h"
#include <libSUSE/libSUSE.h>
#include <libSUSE/utils.h>

int PID = 0;
int TID = 0;
int GRADO_MULTIPROGRAMACION = 0;

t_log* logger;
t_dictionary* diccionario_procesos;
t_dictionary* diccionario_ults;

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* colas_ready[100];
t_queue* colas_exec[100];

sem_t* tid_sem;
sem_t* pid_sem;
sem_t* multiprogramacion_sem;

void inicializar();
void atender_cliente(int);
void atender_nuevo_cliente(int);
void llega_nuevo_hilo(ult_t*);
void pasar_a_ready();
void liberar();
