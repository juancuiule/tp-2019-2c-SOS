#include "MUSE.h"

void recibir_mensaje(int socket_cliente) {
	int size;

	char* buffer = recv_buffer(&size, socket_cliente);

	log_info(logger, "Me llego el mensaje: %s", buffer);

//	char* respuesta = "Hola, ya te escuche!";
//
//	enviar_mensaje(respuesta, socket_cliente);
	free(buffer);
}




int respond_to_client(int cliente_fd) {
	while(1) {
		int cod_op = recv_muse_op_code(cliente_fd);
		recv_muse_id(cliente_fd);
		switch(cod_op) {
			case INIT_MUSE:
				recibir_mensaje(cliente_fd);
				break;
			case ALLOC:
				log_info(logger, "muse_alloc");
				//realizar cod_op
				break;
			case FREE:
				log_info(logger, "muse_free.");
				//realizar cod_op
				break;
			case GET:
				log_info(logger, "muse_get");
				//realizar cod_op
				break;
			case CPY:
				log_info(logger, "muse_copy.");
				//realizar cod_op
				break;
			case MAP:
				log_info(logger, "muse_map");
				//realizar cod_op
				break;
			case SYNC:
				log_info(logger, "muse_sync");
				//realizar cod_op
				break;
			case UNMAP:
				log_info(logger, "muse_unmap");
				//realizar cod_op
				break;
			case DISCONNECT_MUSE:
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

	int server_fd = init_server(IP, PORT);
	
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
