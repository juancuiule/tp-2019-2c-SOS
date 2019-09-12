#include "server.h"

int main(void) {
	logger = log_create("../logs/server.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = config_create("../program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int currified_log_info(char* message) {
		log_info(logger, message);
	}

	int server_fd = iniciar_servidor(IP, PORT, logger);
	
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	for (;;) {
		int cliente_fd = esperar_cliente(server_fd, (void *) currified_log_info);
		respond_to_client(cliente_fd, logger);
	}
	return EXIT_SUCCESS;
}
