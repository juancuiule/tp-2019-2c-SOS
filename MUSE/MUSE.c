#include "MUSE.h"

void recibir_mensaje(int socket_cliente) {
	int size;

	char* buffer = recibir_buffer(&size, socket_cliente);

	log_info(logger, "Me llego el mensaje: %s", buffer);

	char* respuesta = "Hola, ya te escuche!";

	enviar_mensaje(respuesta, socket_cliente);
	free(buffer);
}

int respond_to_client(int cliente_fd) {
	while(1) {
		int cod_op = recibir_operacion(cliente_fd);
		switch(cod_op) {
			case MENSAJE:
				recibir_mensaje(cliente_fd);
				break;
			case DESCONECTAR:
				log_info(logger, "El cliente se desconecto.");
				return EXIT_FAILURE;
				break;
			default:
				log_warning(logger, "Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

int main(void) {
	logger = log_create("./logs/MUSE.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = config_create("./program.config");

	char* IP = "127.0.0.1";
	char* PORT = config_get_string_value(config, "LISTEN_PORT");

	int server_fd = iniciar_servidor(IP, PORT);
	
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	pthread_t hilo;
	int r1;
	while (1) {
		int cliente_fd = recibir_cliente(server_fd);
	    r1 = pthread_create(&hilo, NULL, (void*) respond_to_client, (void*) cliente_fd);
	}
	return EXIT_SUCCESS;
}
