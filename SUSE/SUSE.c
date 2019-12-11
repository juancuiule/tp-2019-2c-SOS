#include "SUSE.h"
#include "globales.h"

int main() {
	int cliente_fd;
	pthread_t hilo_clientes, hilo_metricas;

	configurar();
	inicializar();
	inicializar_diccionario_semaforos();

	servidor_fd = iniciar_servidor();

	//pthread_create(&hilo_metricas, NULL, (void*)logear_metricas, NULL);

	while(1) {
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo_clientes, NULL, (void*)atender_cliente, cliente_fd);
		// TODO: ver porque se está cerrando SUSE. OK
	}

	//pthread_join(hilo_metricas, NULL);
	liberar();
	return EXIT_SUCCESS;
}

void imprimir_estados(int pid) {
	programa_t* programa = obtener_programa(pid);
	imprimir_hilos_en_new();
	imprimir_hilos_en_ready(programa);

	if (programa->hilo_en_exec != NULL)
		printf("EXEC: %i\n", programa->hilo_en_exec->tid);

	imprimir_hilos_en_blocked();
	imprimir_hilos_en_exit();
}

void imprimir_hilos_en_new() {
	printf("NEW: ");

	bool imprimir_tid(hilo_t* hilo) {
		printf("%i ", hilo->tid);
	}

	list_iterate(cola_new->elements, imprimir_tid);
	printf("\n");
}

void imprimir_hilos_en_ready(programa_t* programa) {
	printf("READY: ");

	bool imprimir_tid(hilo_t* hilo) {
		printf("%i ", hilo->tid);
	}

	list_iterate(programa->hilos_en_ready, imprimir_tid);
	printf("\n");
}

void imprimir_hilos_en_blocked() {
	printf("BLOCKED: ");

	bool imprimir_tid(hilo_t* hilo) {
		printf("%i ", hilo->tid);
	}

	list_iterate(cola_blocked, imprimir_tid);
	printf("\n");
}

bool esta_bloqueado(hilo_t* hilo) {

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return hilo->tid == un_hilo->tid && hilo->pid == un_hilo->pid;
	}

	return list_any_satisfy(cola_blocked, es_hilo_buscado);
}

void imprimir_hilos_en_exit() {
	printf("EXIT: ");

	bool imprimir_tid(hilo_t* hilo) {
		printf("%i ", hilo->tid);
	}

	list_iterate(cola_exit, imprimir_tid);
	printf("\n");
}

bool esta_en_ready(programa_t* programa, hilo_t* hilo) {

	bool hilo_encontrado(hilo_t* un_hilo) {
		return hilo->tid == un_hilo->tid && hilo->pid == un_hilo->pid;
	}

	return list_any_satisfy(programa->hilos_en_ready, hilo_encontrado);
}

uint64_t tiempo_actual() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	unsigned long long tiempo = (((unsigned long long )tv.tv_sec) * 1000 + ((unsigned long) tv.tv_usec) / 1000);
	return (uint64_t)tiempo;
}

void inicializar_metricas_hilo(hilo_t* hilo) {
	hilo->tiempo_creacion = 0;
	hilo->tiempo_cpu = 0;
	hilo->tiempo_espera = 0;
}

bool es_programa_hilo_buscado(programa_t* programa) {
	return programa->hilo_en_exec->tid == tid_hilo_buscado;
}

programa_t* obtener_programa_de_hilo(int tid) {
	tid_hilo_buscado = tid;
	return list_find(programas, (void*)es_programa_hilo_buscado);
}

hilo_t* crear_hilo(int pid, int tid) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->pid = pid;
	hilo->tid = tid;
	hilo->tiempo_cpu = 0;
	hilo->tiempo_creacion = 0;
	hilo->tiempo_espera = 0;
	hilo->tiempo_ultima_llegada_a_ready = 0;
	hilo->estimacion_anterior = 0;
	hilo->rafaga_anterior = 0;
	hilo->tid_hilo_a_esperar = 0;
	return hilo;
}

void sacar_de_ready(programa_t* programa, hilo_t* hilo) {
	bool es_hilo_buscado(hilo_t* un_hilo) {
		return hilo->tid == un_hilo->tid && hilo->pid == un_hilo->pid;
	}

	list_remove_by_condition(programa->hilos_en_ready, es_hilo_buscado);
}

bool finalizado(hilo_t* hilo) {

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return hilo->tid == un_hilo->tid && hilo->pid == un_hilo->pid;
	}

	return list_any_satisfy(cola_exit, es_hilo_buscado);
}

void desbloquear_hilo(hilo_t* hilo) {

	bool es_hilo_buscado(hilo_t* un_hilo) {
		return hilo->tid == un_hilo->tid && hilo->pid == un_hilo->pid;
	}

	list_remove_by_condition(cola_blocked, es_hilo_buscado);
	encolar_hilo_en_ready(hilo);
}

hilo_t* obtener_hilo(t_list* lista_de_hilos, int pid, int tid) {

	bool es_hilo_buscado(hilo_t* hilo) {
		return hilo->pid == pid && hilo->tid == tid;
	}

	return list_find(lista_de_hilos, es_hilo_buscado);
}

void atender_cliente(int cliente_fd) {
	int pedido;
	int valor_semaforo;
	int offset;
	int opcode;
	int size;
	int tid;
	int tid_proximo_hilo;
	int tid_hilo_a_esperar;
	int tid_hilo_esperando;
	int tid_hilo_a_bloquear;
	int pid;
	int tamanio;
	int tamanio_id_semaforo;
	int socket_esta_conectado;
	char* id_semaforo;
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo_t* hilo_a_bloquear = malloc(sizeof(hilo_t));
	hilo_t* hilo_a_desbloquear = malloc(sizeof(hilo_t));
	hilo_t* proximo_hilo = malloc(sizeof(hilo_t));
	hilo_t* hilo_esperando = malloc(sizeof(hilo_t));
	hilo_t* hilo_a_esperar = malloc(sizeof(hilo_t));
	programa_t* programa = malloc(sizeof(programa_t));
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa->hilos_en_ready = list_create();

	socket_esta_conectado = recv(cliente_fd, &opcode, sizeof(int), MSG_WAITALL);
	recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);

	while(socket_esta_conectado > 0) {
		id_semaforo = string_new();
		//TODO: salir del while al recibir close
		switch (opcode) {
			case INIT:
				agregar_programa(pid);
				log_info(logger, "Llegó el programa %i", pid);
				imprimir_estados(pid);
				break;
			case CREATE:
				recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
				hilo = crear_hilo(pid, tid);
				encolar_hilo_en_new(hilo);

				if (GRADO_MULTIPROGRAMACION < MAX_MULTIPROG) {
					hilo = queue_pop(cola_new);
					encolar_hilo_en_ready(hilo);
					pthread_mutex_lock(&mutex_multiprogramacion);
					GRADO_MULTIPROGRAMACION++;
					pthread_mutex_unlock(&mutex_multiprogramacion);
				}

				imprimir_estados(pid);
				break;
			case SCHEDULE_NEXT:
				//tomar el hilo en EXEC y mandarlo a READY. OK
				programa = obtener_programa(pid);
				hilo = crear_hilo(pid, tid);

				if (programa->hilo_en_exec != NULL)
					list_add(programa->hilos_en_ready, programa->hilo_en_exec);

				proximo_hilo = siguiente_hilo_a_ejecutar(pid);
				sacar_de_ready(programa, proximo_hilo);
				programa->hilo_en_exec = proximo_hilo;
				tid_proximo_hilo = proximo_hilo->tid;

				if (!finalizado(proximo_hilo))
					log_info(logger, "El hilo %i del programa %i llegó a EXEC", tid_proximo_hilo, pid);

				imprimir_estados(pid);
				send(cliente_fd, &tid_proximo_hilo, sizeof(int), MSG_WAITALL);
				break;
			case JOIN:
				recv(cliente_fd, &tid_hilo_a_bloquear, sizeof(int), MSG_WAITALL);
				recv(cliente_fd, &tid_hilo_a_esperar, sizeof(int), MSG_WAITALL);
				hilo_a_bloquear = crear_hilo(pid, tid_hilo_a_bloquear);
				hilo_a_bloquear->tid_hilo_a_esperar = tid_hilo_a_esperar;
				hilo_a_esperar = crear_hilo(pid, tid_hilo_a_esperar);
				programa = obtener_programa(pid);

				if (!esta_bloqueado(hilo_a_bloquear) && !finalizado(hilo_a_esperar)) {
					sacar_de_ready(programa, hilo_a_bloquear);
					bloquear_hilo(hilo_a_bloquear);
				}

				imprimir_estados(pid);
				break;
			case CLOSE:
				recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
				hilo = crear_hilo(pid, tid);
				programa_t* programa = obtener_programa(pid);
				cerrar_hilo(hilo);
				hilo_esperando = crear_hilo(hilo->pid, hilo->tid_hilo_a_esperar);
				desbloquear_hilo(hilo_esperando);
				imprimir_estados(pid);
				break;
			case WAIT:
				recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
				recv(cliente_fd, &tamanio_id_semaforo, sizeof(int), MSG_WAITALL);
				recv(cliente_fd, id_semaforo, tamanio_id_semaforo, 0);
				valor_semaforo = semaforo_wait(id_semaforo);

				if (valor_semaforo == 0) {
					hilo_a_bloquear = crear_hilo(pid, tid);
					log_info(logger, "El hilo %i hizo un wait al semáforo %s (valor actual = %i)", tid, id_semaforo, valor_semaforo);
					bloquear_hilo(hilo_a_bloquear);
				}
				else
					log_info(logger, "El hilo %i hizo un wait al semáforo %s (valor actual = %i)", tid, id_semaforo, valor_semaforo);

				imprimir_estados(pid);
			break;
			case SIGNAL:
				recv(cliente_fd, &tid, sizeof(int), MSG_WAITALL);
				recv(cliente_fd, &tamanio_id_semaforo, sizeof(int), MSG_WAITALL);
				recv(cliente_fd, id_semaforo, tamanio_id_semaforo, 0);
				valor_semaforo = semaforo_signal(id_semaforo);
				log_info(logger, "El hilo %i hizo un signal al semáforo %s (valor actual = %i)", tid, id_semaforo, valor_semaforo);
				hilo_a_desbloquear = crear_hilo(pid, tid);

				if (esta_bloqueado(hilo))
					desbloquear_hilo(hilo_a_desbloquear);

				imprimir_estados(pid);
			break;
		}

		socket_esta_conectado = recv(cliente_fd, &opcode, sizeof(int), MSG_WAITALL);
		recv(cliente_fd, &pid, sizeof(int), MSG_WAITALL);
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

	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	hilo_t* hilo_anterior = malloc(sizeof(hilo_t));
	programa_t* programa = malloc(sizeof(programa_t));
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	if (programa->hilo_en_exec != NULL)
		hilo_anterior = programa->hilo_en_exec;

	hilo_t* siguiente_hilo = siguiente_hilo_a_ejecutar(programa);

	sacar_de_ready(programa, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a EXEC.", siguiente_hilo->tid, siguiente_hilo->pid);
	siguiente_hilo->tiempo_ultima_llegada_a_exec = tiempo_actual();
	siguiente_hilo->tiempo_espera += tiempo_actual() - hilo->tiempo_ultima_llegada_a_ready;
	programa->hilo_en_exec = siguiente_hilo;
	list_remove_by_condition(programa->hilos_en_ready, hilo_encontrado);
}

void agregar_programa(int pid) {
	programa_t* programa = malloc(sizeof(programa_t));
	programa->pid = pid;
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = NULL;
	list_add(programas, programa);
}

void atender_nuevo_cliente(int cliente_fd) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo = recibir_paquete(cliente_fd);
}

void encolar_hilo_en_new(hilo_t* hilo) {
	programa_t* programa = obtener_programa(hilo->pid);
	hilo->tiempo_creacion = tiempo_actual();
	queue_push(cola_new, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a NEW", hilo->tid, hilo->pid);
}

void cerrar_hilo(hilo_t* hilo) {
	programa_t* programa = obtener_programa(hilo->pid);
	programa->hilo_en_exec = NULL;
	list_add(cola_exit, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a EXIT", hilo->tid, hilo->pid);
	pthread_mutex_lock(&mutex_multiprogramacion);
	GRADO_MULTIPROGRAMACION--;
	pthread_mutex_unlock(&mutex_multiprogramacion);
}

void encolar_hilo_en_ready(hilo_t* hilo) {

	bool hilo_encontrado(hilo_t* un_hilo) {
		return un_hilo->tid == hilo->tid && un_hilo->pid == hilo->pid;
	}

	programa_t* programa = malloc(sizeof(programa_t));
	programa->hilos_en_ready = list_create();
	programa->hilo_en_exec = malloc(sizeof(hilo_t));
	programa = obtener_programa(hilo->pid);

	if (list_any_satisfy(cola_blocked, hilo_encontrado))
		list_remove_by_condition(cola_blocked, hilo_encontrado);

	if (!finalizado(hilo)) {

		if (!esta_en_ready(programa, hilo))
			list_add(programa->hilos_en_ready, hilo);

		hilo->tiempo_ultima_llegada_a_ready = tiempo_actual();
		log_info(logger, "El hilo %i del programa %i llegó a READY", hilo->tid, hilo->pid);

		if (GRADO_MULTIPROGRAMACION == MAX_MULTIPROG)
			log_warning(logger, "Se ha alcanzado el grado máximo de multiprogramación");
	}

}

void bloquear_hilo(hilo_t* hilo) {
	programa_t* programa_del_hilo = obtener_programa(hilo->pid);
	programa_del_hilo->hilo_en_exec = NULL;
	list_add(cola_blocked, hilo);
	log_info(logger, "El hilo %i del programa %i llegó a BLOCKED", hilo->tid, hilo->pid);
}

hilo_t* siguiente_hilo_a_ejecutar(int pid) {
	hilo_t* siguiente = malloc(sizeof(hilo_t));
	t_list* hilos = list_create();
	programa_t* programa = obtener_programa(pid);
	hilos = programa->hilos_en_ready;

	long long estimacion(hilo_t* hilo) {
		long long estimacion = (1 - ALPHA_SJF) * hilo->estimacion_anterior + ALPHA_SJF * hilo->rafaga_anterior;
		 return estimacion;
	}

	bool comparador(hilo_t* hilo1, hilo_t* hilo2) {
		return estimacion(hilo1) <= estimacion(hilo2);
	}

	list_sort(hilos, comparador);

	if (!list_is_empty(hilos)) {
		siguiente = list_get(hilos, 0);
		siguiente->estimacion_anterior = estimacion(siguiente);
	}
	return siguiente;
}

void liberar() {

	void liberar_hilo(hilo_t* hilo) {
		free(hilo);
	}

	void liberar_programa(programa_t* programa) {
		free(programa);
	}

	void liberar_hilos_programa(programa_t* programa) {
		list_destroy(programa->hilos_en_ready);
		free(programa->hilo_en_exec);
	}

	config_destroy(config);
	log_destroy(logger);
	log_destroy(logger_metricas);
	list_iterate(programas, liberar_hilos_programa);
	list_destroy_and_destroy_elements(programas, liberar_programa);
	list_destroy_and_destroy_elements(cola_new->elements, liberar_hilo);
	list_destroy_and_destroy_elements(cola_blocked, liberar_hilo);
	list_destroy_and_destroy_elements(cola_exit, liberar_hilo);
	dictionary_destroy(diccionario_semaforos);
}


















