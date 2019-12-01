#include "SUSE.h"
#include "globales.h"

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
		pthread_detach(hilo_clientes);
	}

	pthread_detach(hilo_metricas);
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
	hilo->hilos_a_esperar= queue_create();
}

bool es_programa_hilo_buscado(programa_t* programa) {
	return programa->hilo_en_exec->tid == tid_hilo_buscado;
}

programa_t* obtener_programa_de_hilo(int tid) {
	tid_hilo_buscado = tid;
	return list_find(programas, (void*)es_programa_hilo_buscado);
}

void eliminar_hilo_de_hilos_a_esperar(hilo_t* hilo_que_espera, int tid) {

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return un_hilo->tid == tid;
	}

	if (!list_is_empty(hilo_que_espera))
		list_remove_by_condition(hilo_que_espera->hilos_a_esperar, es_hilo_buscado);
}

bool no_espera_mas_hilos(hilo_t* hilo) {
	return list_is_empty(hilo->hilos_a_esperar);
}

void validar_si_esta_esperando_hilos(hilo_t* hilo) {
	if (list_is_empty(hilo->hilos_a_esperar))
		encolar_hilo_en_ready(hilo);
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
	char* proximo;

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid;
	}

	programa_t* programa = malloc(sizeof(programa_t));
	programa = obtener_programa(pid);
	hilo_t* proximo_hilo = malloc(sizeof(hilo_t));

	switch (opcode) {
		case 1:
			encolar_hilo_en_new(hilo);
			agregar_programa(hilo);

			if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG) {
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
			proximo = string_itoa(proximo_hilo->tid);
			send(cliente_fd, proximo, sizeof(proximo), MSG_WAITALL);
			free(proximo);
			break;
		case 3:
			if (!list_any_satisfy(cola_blocked->elements, es_hilo_buscado))
				bloquear_hilo(hilo);

			break;
		case 4:
			cerrar_hilo(hilo);

			if (!queue_is_empty(cola_blocked)) {
				list_iterate(cola_blocked->elements, eliminar_hilo_de_hilos_a_esperar);
				list_iterate(cola_blocked->elements, validar_si_esta_esperando_hilos);
			}

			ejecutar_nuevo_hilo(hilo);

			break;
	}

	free(proximo_hilo);
	free(buffer);
}

bool es_programa_buscado(programa_t* programa) {
	return programa->pid == pid_programa_buscado;
}

programa_t* obtener_programa(int pid) {
	pid_programa_buscado = pid;
	return list_find(programas, (void*)es_programa_buscado);
}

void ejecutar_nuevo_hilo(hilo_t* hilo) {

	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid;
	}

	hilo_t* hilo_anterior = malloc(sizeof(hilo_t));
	programa_t* programa = malloc(sizeof(programa_t));
	programa->cola_ready = queue_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	if (programa->hilo_en_exec != NULL)
		hilo_anterior = programa->hilo_en_exec;

	hilo_t* siguiente_hilo = siguiente_hilo_a_ejecutar(programa);

	log_info(logger, "El hilo %i del programa %i llegó a EXEC.", siguiente_hilo->tid, siguiente_hilo->pid);
	siguiente_hilo->tiempo_ultima_llegada_a_exec = current_timestamp();
	siguiente_hilo->tiempo_espera += current_timestamp() - hilo->tiempo_ultima_llegada_a_ready;
	programa->hilo_en_exec = siguiente_hilo;
	list_remove_by_condition(programa->cola_ready->elements, hilo_encontrado);
	free(hilo_anterior);
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
	programa_t* programa = obtener_programa(hilo->pid);

	queue_push(cola_exit, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a EXIT.", hilo->tid, hilo->pid);
	sem_wait(multiprogramacion_sem);
	GRADO_MULTIPROGRAMACION--;
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
	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid;
	}

	programa_t* programa_del_hilo = obtener_programa(hilo->pid);
	list_remove_by_condition(programa_del_hilo->cola_ready->elements, hilo_encontrado);
	queue_push(cola_blocked, hilo);
	log_info(logger, "El hilo %d del programa %i llegó a BLOCKED", hilo->tid, hilo->pid);
}

hilo_t* siguiente_hilo_a_ejecutar(programa_t* programa) {
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

	void liberar_programa(programa_t* programa) {
		free(programa);
	}

	void liberar_hilos_programa(programa_t* programa) {
		queue_destroy(programa->cola_ready);
		free(programa->hilo_en_exec);
	}

	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	sem_destroy(tid_sem);
	sem_destroy(pid_sem);
	sem_destroy(multiprogramacion_sem);
	list_iterate(programas, liberar_hilos_programa);
	list_destroy_and_destroy_elements(programas, liberar_programa);
}


















