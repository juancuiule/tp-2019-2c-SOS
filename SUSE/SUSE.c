#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo_clientes, hilo_metricas;

	inicializar();
	configurar();
	int servidor_fd = iniciar_servidor();

	pthread_create(&hilo_metricas, NULL, logear_metricas, NULL);

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo_clientes, NULL, atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

void inicializar() {
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	diccionario_procesos = dictionary_create();
	diccionario_tid_pid = dictionary_create();
	diccionario_tid = dictionary_create();

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

void logear_metricas() {
	while (1) {
		sleep(METRICS_TIMER);
		log_info(logger_metricas, "Grado de multiprogramación: %d", GRADO_MULTIPROGRAMACION);
	}
}

void atender_cliente(int cliente_fd) {
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	mensaje = recibir_paquete(cliente_fd);

	switch (mensaje->operacion) {
		case 1:
			llega_nuevo_hilo(mensaje->ult);
			break;
		case 3:
			bloquear_hilo(mensaje->ult);
			break;
		case 4:
			cerrar_hilo(mensaje->ult);
			break;
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
	log_info(logger, "El ULT %d ha llegado a la cola de NEW", TID);

	sprintf(pid, "%d", ult->pid);
	sprintf(tid, "%d", ult->tid);

	if(!dictionary_has_key(diccionario_procesos, pid)) {
		dictionary_put(diccionario_procesos, pid, PID);
		dictionary_put(diccionario_tid_pid, tid, PID);
		sem_wait(pid_sem);
		PID++;
		sem_post(pid_sem);
	}

	if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG) {
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación.");
	}
	else {
		ult = queue_pop(cola_new);
		queue_push(colas_ready[PID], ult);
		log_info(logger, "El ULT %d del proceso %d ha llegado a la cola de READY.", TID, PID);
	}

	sem_wait(tid_sem);
	TID++;
	sem_post(tid_sem);

	if (GRADO_MULTIPROGRAMACION < 10) {
		sem_wait(multiprogramacion_sem);
		GRADO_MULTIPROGRAMACION++;
		sem_post(multiprogramacion_sem);
	}
}

void cerrar_hilo(ult_t* ult) {
	queue_push(cola_exit, ult);
	log_info(logger, "El ULT %i ha llegado a la cola de EXIT.", ult->tid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
}

void pasar_a_ready() {
	int tid = 0;
	tid = queue_pop(cola_new);
	char* thread_id = malloc(10);
	sprintf(tid, thread_id);
	int indice = dictionary_get(diccionario_tid_pid, thread_id);
	queue_push(colas_ready[indice], tid);
	printf("El ULT %i pasó a READY\n", tid);
}

void bloquear_hilo(ult_t* ult) {
	char* thread_id = string_itoa(ult->tid);
	int pid = dictionary_get(diccionario_tid_pid, thread_id);
	ult_t* thread = queue_pop(colas_exec[pid]);
	queue_push(cola_blocked, thread);
	log_info(logger, "El thread %d ha llegado a la cola de BLOCKED", dictionary_get(diccionario_tid, thread_id));
}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	dictionary_destroy(diccionario_procesos);
	dictionary_destroy(diccionario_tid_pid);
	dictionary_destroy(diccionario_tid);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
}
















