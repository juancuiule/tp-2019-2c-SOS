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

	tid_sem = malloc(sizeof(sem_t));
	sem_init(tid_sem, 0 , 1);

	pid_sem = malloc(sizeof(sem_t));
	sem_init(pid_sem, 0, 1);

	multiprogramacion_sem = malloc(sizeof(sem_t));
	sem_init(multiprogramacion_sem, 0, 1);
}

void atender_cliente(int cliente_fd) {
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	mensaje = recibir_paquete(cliente_fd);

	if (mensaje->operacion == 1)
		llega_nuevo_hilo(mensaje->ult);

	if (mensaje->operacion == 4) {
		cerrar_hilo(mensaje->ult);
		printf("operacion cerrar hilo\n");
	}

}

void atender_nuevo_cliente(int cliente_fd) {
	ult_t* ult = malloc(sizeof(ult_t));
	ult = recibir_paquete(cliente_fd);
	llega_nuevo_hilo(ult);
}

void llega_nuevo_hilo(ult_t* ult) {
	char* pid = malloc(10);
	char* tid = malloc(10);

	queue_push(cola_new, ult);
	log_info(logger, "El ULT %d ha llegado a la cola de NEW\n", TID);

	sprintf(pid, "%d", ult->pid);

	if(!dictionary_has_key(diccionario_procesos, pid)) {
		dictionary_put(diccionario_procesos, pid, PID);
		dictionary_put(diccionario_ults, tid, PID);
		sem_wait(pid_sem);
		PID++;
		sem_post(pid_sem);
	}

	if (GRADO_MULTIPROGRAMACION >= MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación.\n");
	else {
		ult = queue_pop(cola_new);
		queue_push(colas_ready[PID], ult);
		log_info(logger, "El ULT %d del proceso %d ha llegado a la cola de READY\n", TID, PID);
	}

	sem_wait(tid_sem);
	TID++;
	sem_post(tid_sem);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION++;
	sem_post(multiprogramacion_sem);
}

void cerrar_hilo(ult_t* ult) {
	queue_push(cola_exit, ult);
	log_info(logger, "El ULT %d ha llegado a la cola de EXIT\n", ult->tid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
}

void pasar_a_ready() {
	int tid = 0;
	tid = queue_pop(cola_new);
	char* thread_id = malloc(10);
	sprintf(tid, thread_id);
	int indice = dictionary_get(diccionario_ults, thread_id);
	queue_push(colas_ready[indice], tid);
	printf("El ULT %i pasó a READY\n", tid);
}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	dictionary_destroy(diccionario_procesos);
	dictionary_destroy(diccionario_ults);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
}
















