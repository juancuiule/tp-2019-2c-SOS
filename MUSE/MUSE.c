#include "MUSE.h"

void* MEMORIA;

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
	
	muse_body* r_body = create_empty_body();
	add_fixed_to_body(r_body, sizeof(uint32_t), MEMORIA);
	muse_response* response = create_response(SUCCESS, r_body);
	send_response(response, socket_cliente);
}

void respond_get(muse_body* body, char* id, int socket_cliente) {
	uint32_t src;
	memcpy(&src, body->content, sizeof(uint32_t));
	size_t size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(size_t));

	log_info(logger, "El cliente con id: %s hizo get a la src: %u de %i bytes", id, src, size);

	void* val = malloc(size);
	memcpy((void*) val, MEMORIA, size);

	log_info(logger, "El valor %s", (char*) val);

	muse_body* r_body = create_empty_body();
	add_to_body(r_body, size, val);
	muse_response* response = create_response(SUCCESS, r_body);
	send_response(response, socket_cliente);
}

void respond_free(muse_body* body, char* id, int socket_cliente) {
	uint32_t dir_to_free;
	memcpy(&dir_to_free, body->content, sizeof(uint32_t));
	log_info(logger, "El cliente con id: %s hizo free a la dir: %u", id, dir_to_free);

	free(dir_to_free);
	muse_body* r_body = create_empty_body();
	muse_response* response = create_response(SUCCESS, r_body);
	send_response(response, socket_cliente);
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
				respond_free(body, id, cliente_fd);
				break;
			case GET:
				log_info(logger, "muse_get.");
				respond_get(body, id, cliente_fd);
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
