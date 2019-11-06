#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>

#include "utils.h"
#include "configuracion.h"

int PID = 0;
int TID = 0;

t_dictionary* diccionario_procesos;
t_dictionary* diccionario_ults;

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* colas_ready[100];
t_queue* colas_exec[100];

sem_t* tid_inc_sem;

void inicializar();
void atender_cliente(int);
void llega_nuevo_hilo(int, int);
void pasar_a_ready();
void liberar();
