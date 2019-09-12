#include "server.h"

int main(void) {
	logger = log_create("../logs/server.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = config_create("../program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int server_fd = iniciar_servidor(IP, PORT);
	
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	for (;;) {
		int cliente_fd = esperar_cliente(server_fd);
		respond_to_client(cliente_fd);
	}
	return EXIT_SUCCESS;
}
