#include <stdio.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include "server.h"

t_queue* cola_new;
t_queue* cola_blocked;
t_queue* cola_exit;
