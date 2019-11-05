#include <stdio.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "utils.h"
#include "configuracion.h"

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;

void inicializar();
int atender_cliente(int);
