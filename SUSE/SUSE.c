#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo_clientes, hilo_metricas;

	inicializar();
	configurar();
	servidor_fd = iniciar_servidor();

	pthread_create(&hilo_metricas, NULL, (void*)logear_metricas, NULL);

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo_clientes, NULL, (void*)atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL);
    long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000;
    return milliseconds;
}

void inicializar_metricas_hilo(hilo_t* hilo) {
	hilo->tiempo_creacion = current_timestamp();
	hilo->tiempo_cpu = 0;
	hilo->tiempo_espera = 0;
	hilo->hilos_esperando = queue_create();
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

bool es_programa_hilo_buscado(programa_t* programa) {
	return programa->hilo_en_exec->tid == tid_hilo_buscado;
}

programa_t* obtener_programa_de_hilo(int tid) {
	tid_hilo_buscado = tid;
	return list_find(programas, (void*)es_programa_hilo_buscado);
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
	inicializar_metricas_hilo(hilo);
	hilo->tid = tid;
	hilo->pid = pid;

	programa_t* programa = malloc(sizeof(programa_t));
	programa = obtener_programa(pid);
	hilo_t* proximo_hilo = malloc(sizeof(hilo_t));

	switch (opcode) {
		case 1:
			encolar_hilo_en_new(hilo);
			agregar_programa(hilo);

			if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG) {
				hilo_t* hilo = malloc(sizeof(hilo_t));
				hilo = queue_pop(cola_new);
				encolar_hilo_en_ready(hilo);
			}

			break;
		case 2:

			if (hilo->tid != tid_hilo_anterior) {
				tid_hilo_anterior = hilo->tid;
				ejecutar_nuevo_hilo(hilo);
			}

			hilo_t* proximo_hilo = siguiente_hilo_a_ejecutar(programa);
			char* proximo = string_itoa(proximo_hilo->tid);
			send(cliente_fd, proximo, sizeof(proximo), MSG_WAITALL);
			break;
		case 3:
			bloquear_hilo(hilo);
			break;
		case 4:
			cerrar_hilo(hilo);
			break;
	}

	free(proximo_hilo);
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
	log_info(logger, "El hilo %i del programa %i llegó a EXEC.", hilo->tid, hilo->pid);
	hilo->tiempo_ultima_llegada_a_exec = current_timestamp();
	hilo->tiempo_espera += current_timestamp() - hilo->tiempo_ultima_llegada_a_ready;
	programa->hilo_en_exec = hilo;
}

void logear_metricas() {
	long long tiempo_de_ejecucion(hilo_t* hilo) {
		return current_timestamp() - hilo->tiempo_creacion;
	}

	void logear_metricas_hilo(hilo_t* hilo) {
		log_info(logger_metricas, "Métricas del hilo %i: ", hilo->tid);
		long long tiempo_ejecucion = tiempo_de_ejecucion(hilo);
		log_info(logger_metricas, "\ttiempo de ejecución: %lld ms", tiempo_ejecucion);
		log_info(logger_metricas, "\ttiempo de espera: %lld ms", hilo->tiempo_espera);
		log_info(logger_metricas, "\ttiempo de uso de CPU: %lld ms", hilo->tiempo_cpu);
	}

	void logear_metricas_hilos_programa(programa_t* programa) {
		hilo_t* hilo_en_ejecucion = malloc(sizeof(hilo_t));
		hilo_en_ejecucion = programa->hilo_en_exec;
		t_list* hilos_programa = list_create();
		list_add(hilos_programa, hilo_en_ejecucion);
		list_add_all(hilos_programa, programa->cola_ready->elements);
		list_iterate(hilos_programa, (void*)logear_metricas_hilo);
	}

	while (1) {
		sleep(METRICS_TIMER);
		log_info(logger_metricas, "Grado de multiprogramación: %i", GRADO_MULTIPROGRAMACION);
		list_iterate(cola_new->elements, (void*)logear_metricas_hilo);
		list_iterate(cola_blocked->elements, (void*)logear_metricas_hilo);
		//list_iterate(programas, (void*)logear_metricas_hilos_programa);
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
	hilo->tiempo_creacion = current_timestamp();
	queue_push(cola_new, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a NEW", hilo->tid, hilo->pid);
}

void cerrar_hilo(hilo_t* hilo) {
	queue_push(cola_exit, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a EXIT.", hilo->tid, hilo->pid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
	sem_post(multiprogramacion_sem);
	hilo_t* hilo_esperando = queue_pop(hilo->hilos_esperando);

	while (hilo_esperando != NULL) {
		printf("hilo que estaba esperando: %i\n", hilo_esperando->tid);
		encolar_hilo_en_ready(hilo_esperando);
		hilo_esperando = queue_pop(hilo->hilos_esperando);
	}
}

void encolar_hilo_en_ready(hilo_t* hilo) {
	programa_t* programa = malloc(sizeof(programa_t));
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);
	queue_push(programa->cola_ready, hilo);
	hilo->tiempo_ultima_llegada_a_ready = current_timestamp();
	log_info(logger, "El hilo %d del programa %d llegó a READY", hilo->tid, hilo->pid);
	GRADO_MULTIPROGRAMACION++;

	if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG)
		log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación");

}

void bloquear_hilo(hilo_t* hilo) {
	queue_push(cola_blocked, hilo);
	log_info(logger, "El hilo %d del programa %i llegó a BLOCKED", hilo->tid, hilo->pid);
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

void liberar_hilos_programa(programa_t* programa) {
	queue_destroy(programa->cola_ready);
	free(programa->hilo_en_exec);
}

void liberar() {
	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
	list_iterate(programas, liberar_hilos_programa);
	list_destroy(programas);
}


















