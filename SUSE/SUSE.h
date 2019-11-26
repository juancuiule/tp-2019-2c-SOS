#include <stdio.h>
#include <stdlib.h>
#include <commons/collections/queue.h>
#include <pthread.h>
#include <semaphore.h>
#include "utils.h"
#include "configuracion.h"
#include <commons/string.h>
#include <stdbool.h>

typedef struct {
	int tid;
	int pid;
	int tiempo_ejecucion;
	int tiempo_espera;
	int tiempo_cpu;
	float estimacion_anterior;
	float rafaga_anterior;
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

int servidor_fd;
int pid_programa_buscado;
int tid_siguiente_hilo;

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
programa_t* obtener_programa(int);
void ejecutar_nuevo_hilo(hilo_t*);
void liberar();
