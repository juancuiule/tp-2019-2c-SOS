#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo;

	inicializar();
	configurar();
	int servidor_fd = iniciar_servidor();

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo, NULL, atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

void inicializar() {
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	diccionario_procesos = dictionary_create();
	diccionario_ults = dictionary_create();

	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();

	for (int i = 0; i < 100; i++) {
		colas_ready[i] = queue_create();
		colas_exec[i] = queue_create();
	}

	tid_inc_sem = malloc(sizeof(sem_t));
	sem_init(tid_inc_sem, 0 , 1);

	pid_inc_sem = malloc(sizeof(sem_t));
	sem_init(pid_inc_sem, 0, 1);
}

void atender_cliente(int cliente_fd) {
	ult_t* ult = malloc(sizeof(ult_t));
	ult = recibir_paquete(cliente_fd);
	llega_nuevo_hilo(ult);
}

void llega_nuevo_hilo(ult_t* ult) {
	char* pid = malloc(10);
	char* tid = malloc(10);

	queue_push(cola_new, ult->tid);
	log_info(logger, "El ULT %d ha llegado a la cola de NEW\n", TID);

	sprintf(pid, "%d", ult->pid);

	if(!dictionary_has_key(diccionario_procesos, pid)) {
		dictionary_put(diccionario_procesos, pid, PID);
		dictionary_put(diccionario_ults, tid, PID);
		sem_wait(pid_inc_sem);
		PID++;
		sem_post(pid_inc_sem);
	}

	if (GRADO_MULTIPROGRAMACION >= MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación.\n");
	else {
		queue_push(colas_ready[PID], TID);
		log_info(logger, "El ULT %d del proceso %d ha llegado a la cola de READY\n", TID, PID);
	}

	sem_wait(tid_inc_sem);
	TID++;
	GRADO_MULTIPROGRAMACION++;
	sem_post(tid_inc_sem);
}

void pasar_a_ready() {
	int tid = 0;
	tid = queue_pop(cola_new);
	/*
	char* pid = malloc(3);
	sprintf(pid, dictionary_get(diccionario_ults, tid));
	int indice = dictionary_get(diccionario_procesos, pid);
	queue_push(colas_ready[indice], ult->tid);
	printf("El ULT %i paso a ready\n", ult->tid);
	*/
	char* thread_id = malloc(10);
	sprintf(tid, thread_id);
	int indice = dictionary_get(diccionario_ults, thread_id);
	queue_push(colas_ready[indice], tid);
	printf("El ULT %i paso a READY\n", tid);
}

int siguiente_ult_a_ejecutar(int pid) {
	int* ults_de_proceso = malloc(sizeof(int));

}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	dictionary_destroy(diccionario_procesos);
	dictionary_destroy(diccionario_ults);
	sem_destroy(tid_inc_sem);
	sem_destroy(pid_inc_sem);
}





















