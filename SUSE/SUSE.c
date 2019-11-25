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
		pthread_create(&hilo_clientes, NULL, (void*)atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

void inicializar() {
	logger = log_create("../SUSE.log", "SUSE", 1, LOG_LEVEL_DEBUG);
	logger_metricas = log_create("../METRICAS.log", "SUSE", 1, LOG_LEVEL_DEBUG);

	programas = list_create();
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();

	tid_sem = malloc(sizeof(sem_t));
	sem_init(tid_sem, 0 , 1);

	pid_sem = malloc(sizeof(sem_t));
	sem_init(pid_sem, 0, 1);

	multiprogramacion_sem = malloc(sizeof(sem_t));
	sem_init(multiprogramacion_sem, 0, 1);
}

void atender_cliente(int cliente_fd) {
	int pedido;
	int offset = 0;
	int opcode, size, tid, pid;
	int tamanio;
	opcode = recibir_cod_op(cliente_fd);
	void* buffer = recibir_buffer(&size, cliente_fd);
	memcpy(&tamanio, buffer + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&tid, buffer + offset, sizeof(int));
	offset += sizeof(int);
	memcpy(&pid, buffer + offset, sizeof(int));
	offset += sizeof(int);
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->tid = tid;
	hilo->pid = pid;

	programa_t* programa = malloc(sizeof(programa_t));
	programa = obtener_programa(pid);
	hilo_t* proximo_hilo = malloc(sizeof(hilo_t));

	switch (opcode) {
		case 1:
			encolar_hilo_en_new(hilo);
			agregar_programa(hilo);

			if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG)
				encolar_hilo_en_ready();

			break;
		case 2:
			ejecutar_nuevo_hilo(hilo);
			break;
		case 3:
			bloquear_hilo(hilo);
			break;
		case 4:
			cerrar_hilo(hilo);
			break;
	}

}

bool es_programa_buscado(programa_t* programa) {
	return programa->pid == pid_programa_buscado;
}

programa_t* obtener_programa(int pid) {
	pid_programa_buscado = pid;
	return list_find(programas, (void*)es_programa_buscado);
}

void ejecutar_nuevo_hilo(hilo_t* hilo) {
	hilo_t* hilo_anterior = malloc(sizeof(hilo_t));
	programa_t* programa = malloc(sizeof(programa_t));
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	if (programa->hilo_en_exec != NULL)
		hilo_anterior = programa->hilo_en_exec;

	hilo = siguiente_hilo_a_ejecutar(programa);

	programa->hilo_en_exec = hilo;
	log_info(logger, "El hilo %i del programa %i llegó a EXEC.", hilo->tid, hilo->pid);
}

void logear_metricas() {

	while (1) {
		sleep(METRICS_TIMER);
		log_info(logger_metricas, "Grado de multiprogramación: %i", GRADO_MULTIPROGRAMACION);
	}
}

void agregar_programa(hilo_t* hilo) {
	programa_t* programa = malloc(sizeof(programa_t));
	programa->pid = hilo->pid;
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = NULL;
	list_add(programas, programa);
	PID++;
}

void atender_nuevo_cliente(int cliente_fd) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo = recibir_paquete(cliente_fd);
}

void encolar_hilo_en_new(hilo_t* hilo) {
	queue_push(cola_new, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a NEW", hilo->tid, hilo->pid);
}

void cerrar_hilo(hilo_t* hilo) {
	queue_push(cola_exit, hilo);
	log_info(logger, "El hilo %i ha llegado a la cola de EXIT.", hilo->tid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
}

void encolar_hilo_en_ready() {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo = queue_pop(cola_new);
	programa_t* programa = malloc(sizeof(programa_t));
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);
	queue_push(programa->cola_ready, hilo);
	log_info(logger, "El hilo %d del programa %d está en READY", hilo->tid, hilo->pid);
	GRADO_MULTIPROGRAMACION++;

	if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación");
}

void bloquear_hilo(hilo_t* hilo) {
	programa_t* programa = malloc(sizeof(programa_t));
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);
	queue_push(cola_blocked, hilo);
	log_info(logger, "El hilo %d ha llegado a la cola de BLOCKED", hilo->tid);
}

hilo_t* siguiente_hilo_a_ejecutar(programa_t* programa) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo_t* siguiente = malloc(sizeof(hilo_t));
	t_list* hilos = list_create();
	hilos = programa->cola_ready->elements;

	double estimacion(hilo_t* hilo) {
		return (1 - ALPHA_SJF) * hilo->estimacion_anterior + ALPHA_SJF * hilo->rafaga_anterior;
	}

	bool comparador(hilo_t* hilo1, hilo_t* hilo2) {
		return estimacion(hilo1) <= estimacion(hilo2);
	}

	list_sort(hilos, comparador);
	siguiente = list_get(hilos, 0);

	return siguiente;
}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
	list_destroy(programas);
}
















