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

int atender_cliente(int cliente_fd) {
	printf("Entro al hilo de atender cliente\n");
	ult_t* ult = recibir_paquete(cliente_fd);
	printf("SUSE: atiendo al ULT %i del proceso %i", ult->tid, ult->pid);
}
