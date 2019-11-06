#include "SUSE.h"

int main() {
	int cliente_fd;
	pthread_t hilo;

	configurar();

	int servidor_fd = iniciar_servidor();

	while(1) {
		printf("Entro a while del main\n");
		cliente_fd = esperar_cliente(servidor_fd);
		pthread_create(&hilo, NULL, (void*)atender_cliente, (void*)cliente_fd);
	}

	return EXIT_SUCCESS;
}

void inicializar() {
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
}

int atender_cliente(int cliente_fd) {
	printf("Entro al hilo de atender cliente\n");
	ult_t* ult = recibir_paquete(cliente_fd);
	printf("SUSE: atiendo al ULT %i del proceso %i\n", ult->tid, ult->pid);
}

void planificar_nuevo_hilo(int tid, int pid) {
	if (CANT_ULTS == MAX_MULTIPROG) {
		log_error(logger, "No se puede planificar ULT. Máximo grado de multiprogramación alcanzado.\n");
		return;
	}

	queue_push(cola_new, tid);

	if (dictionary_has_key(diccionario_procesos, atoi(pid)))
		dictionary_put(diccionario_procesos, atoi(pid), PID);

	int process_id = dictionary_get(diccionario_procesos, atoi(pid));
}





















