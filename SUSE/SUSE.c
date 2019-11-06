#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo;

	configurar();
	int servidor_fd = iniciar_servidor();

	while(1) {
		printf("Entro a while del main\n");
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo, NULL, atender_cliente, cliente_fd);
	}

	liberar();
	return EXIT_SUCCESS;
}

void inicializar() {
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
}

void atender_cliente(int cliente_fd) {
	printf("Entro al hilo de atender cliente\n");
	//ult_t* ult = malloc(sizeof(ult_t));
	ult_t* ult = recibir_paquete(cliente_fd);
	printf("SUSE: atiendo al ULT %i del proceso %i\n", ult->tid, ult->pid);
	llega_nuevo_hilo(ult->tid, ult->pid);
}

void llega_nuevo_hilo(int thread_id, int process_id) {
	char* pid = malloc(10);
	char* tid = malloc(10);

	if (CANT_ULTS == MAX_MULTIPROG) {
		log_error(logger, "Se ha alcanzado el grado máximo de multiprogramación.\n");
		return;
	}

	queue_push(cola_new, thread_id);
	printf("El ULT %i ha llegado a la cola de NEW\n", thread_id);
	CANT_ULTS++;

	sprintf(process_id, pid);

	if(!dictionary_has_key(diccionario_procesos, pid)) {
		dictionary_put(diccionario_procesos, pid, PID);
		dictionary_put(diccionario_ults, tid, PID);
		PID++;
	}
}

void pasar_a_ready() {
	int tid = queue_pop(cola_new);
	/*
	char* pid = malloc(3);
	sprintf(pid, dictionary_get(diccionario_ults, tid));
	int indice = dictionary_get(diccionario_procesos, pid);
	queue_push(colas_ready[indice], ult->tid);
	printf("El ULT %i paso a ready\n", ult->tid);
	*/
	char* thread_id = malloc(sizeof(int));
	sprintf(tid, thread_id);
	int indice = dictionary_get(diccionario_ults, thread_id);
	queue_push(colas_ready[indice], tid);
	printf("El ULT %i paso a READY\n", tid);
}





















