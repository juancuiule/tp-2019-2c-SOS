#include "MUSE.h"

void * MEMORIA;

void recv_void_msg(int socket_cliente) {
	int size;
	recv_buffer(&size, socket_cliente);
	// solo para "consumir" el buffer vacio que viene
}

void recibir_mensaje(int socket_cliente) {
	int size;

	char* buffer = recv_buffer(&size, socket_cliente);

	free(buffer);
}

void respond_alloc(muse_body* body, char* id, int socket_cliente) {
	uint32_t tam_pedido;

	memcpy(&tam_pedido, body->content, sizeof(uint32_t));

	log_info(logger, "El cliente con id: %s hizo muse_malloc con %u", id, tam_pedido);
	
	memcpy(MEMORIA, "hola", 5);

	char str[11];
	snprintf(str, sizeof str, "%u", MEMORIA);
	
	muse_response* response = create_response(
		SUCCESS,
		create_body(11, str)
	);
	send_response(response, socket_cliente);
}

void respond_get(int socket_cliente, char* id) {
	int size;

	char* buffer = recv_buffer(&size, socket_cliente);

	char* x;
	void* dir = strtoul(buffer, &x, 10);

	log_info(logger, "El cliente con id: %s hizo get a la dir: %u", id, dir);
	
	muse_response* response = create_response(
		SUCCESS,
		create_body(5, dir)
	);
	send_response(response, socket_cliente);
}

void respond_free(int socket_cliente, char* id) {
	int size;

	char* buffer = recv_buffer(&size, socket_cliente);

	char* x;
	void* dir = strtoul(buffer, &x, 10);

	log_info(logger, "El cliente con id: %s hizo free a la dir: %u", id, dir);
	free(dir);
}


int respond_to_client(int cliente_fd) {
	while(1) {
		int cod_op = recv_muse_op_code(cliente_fd);
		char* id = recv_muse_id(cliente_fd);
		muse_body* body = recv_body(cliente_fd);
		switch(cod_op) {
			case INIT_MUSE:
				// recv_void_msg(cliente_fd);
				break;
			case ALLOC:
				log_info(logger, "muse_alloc.");
				respond_alloc(body, id, cliente_fd);
				break;
			case FREE:
				log_info(logger, "muse_free.");
				// respond_free(cliente_fd, id);
				break;
			case GET:
				log_info(logger, "muse_get.");
				// respond_get(cliente_fd, id);
				break;
			case CPY:
				log_info(logger, "muse_copy.");
				//realizar cod_op
				break;
			case MAP:
				log_info(logger, "muse_map.");
				//realizar cod_op
				break;
			case SYNC:
				log_info(logger, "muse_sync.");
				//realizar cod_op
				break;
			case UNMAP:
				log_info(logger, "muse_unmap.");
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

	MEMORIA = malloc(100);

	pthread_t hilo;
	int r1;
	while (1) {
		int cliente_fd = recibir_cliente(server_fd);
	    r1 = pthread_create(&hilo, NULL, (void*) respond_to_client, (void*) cliente_fd);
	}
	return EXIT_SUCCESS;
}
