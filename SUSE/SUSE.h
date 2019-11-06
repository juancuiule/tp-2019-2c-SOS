#include <stdio.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "utils.h"
#include "configuracion.h"

int PID = 0;
int CANT_ULTS = 0;

t_dictionary* diccionario_procesos;

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
t_queue* colas_ready[100];
t_queue* colas_exec[100];

void inicializar();
int atender_cliente(int);
