#include "SUSE.h"

int main() {
	inicializar_colas();
	configurar();

	int server_fd = init_server(LISTEN_IP, LISTEN_PORT);
	pthread_t hilo;

	while(1) {
		int client_fd = recibir_cliente(server_fd);
		pthread_create(&hilo, NULL, (void*)respond_to_client, (void*)client_fd);
	}

	return 0;
}
