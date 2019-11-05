#include "SUSE.h"

int main() {
	inicializar_colas();
	configurar();

	int servidor_fd = iniciar_servidor(LISTEN_IP, LISTEN_PORT);
	log_info(logger, "Se ha iniciado el servidor.\n");
	pthread_t hilo;

	while(1) {
		int cliente_fd = recibir_cliente(servidor_fd);
		pthread_create(&hilo, NULL, (void*)atender_cliente, (void*)cliente_fd);
	}

	liberar();
	return 0;
}
