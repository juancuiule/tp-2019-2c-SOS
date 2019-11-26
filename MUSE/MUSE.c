#include "MUSE.h"

void respond_init(muse_body* body, char* id, int socket_cliente) {
	create_process_table(id);
}

void respond_alloc(muse_body* body, char* id, int socket_cliente) {
	uint32_t tam_pedido;
	memcpy(&tam_pedido, body->content, sizeof(uint32_t));

	log_info(logger, "El cliente con id: %s hizo muse_malloc con %u", id, tam_pedido);
	process_table* t = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;

	if (t != NULL) {
		int process_segments = t->segments->elements_count;
		log_info(logger, "El proceso %s tiene %i segmentos", id, process_segments);
		process_segment *segment;
		if (process_segments == 0) {
			segment = create_segment(HEAP, 0, tam_pedido);
			int frame_number = find_free_frame(frame_usage_bitmap);
			if (frame_number == -1) {
				log_info(logger, "No hay frame libre... SWAP");
			} else {
				t_page *new_page = create_page(frame_number);
				add_page_to_segment(segment, new_page);
				add_process_segment(id, segment);
			}
			add_fixed_to_body(r_body, sizeof(uint32_t), segment->base);
		} else {
			segment = find_segment_with_space(t->segments, tam_pedido);
			add_fixed_to_body(r_body, sizeof(uint32_t), segment->base);
			// TODO: en vez de devolver la base debería devolver
			//		 el lugar donde puede guardar, el espacio de la página???
		}
		response = create_response(SUCCESS, r_body);
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		response = create_response(ERROR, r_body);
	}
	send_response(response, socket_cliente);
}

void respond_get(muse_body* body, char* id, int socket_cliente) {
	uint32_t src;
	memcpy(&src, body->content, sizeof(uint32_t));
	size_t size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(size_t));

	log_info(logger, "El cliente con id: %s hizo get a la src: %u de %i bytes", id, src, size);
	process_table* t = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;

	if (t != NULL) {
		int process_segments = t->segments->elements_count;
		process_segment *segment;
		if (process_segments == 0) {
			log_info(logger, "El proceso %s no tiene segmentos, es un seg fault?");
			response = create_response(ERROR, r_body);
		} else {
			void* val = malloc(size);
			// TODO: buscar dato en segmento
			memcpy((void*) val, MEMORY, 5);
			add_to_body(r_body, size, val);
			response = create_response(SUCCESS, r_body);
			free(val);
		}
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		response = create_response(ERROR, r_body);
	}
	send_response(response, socket_cliente);
}

void respond_free(muse_body* body, char* id, int socket_cliente) {
	uint32_t dir_to_free;
	memcpy(&dir_to_free, body->content, sizeof(uint32_t));

	// log_info(logger, "El cliente con id: %s hizo free a la dir: %u", id, dir_to_free);

	free(dir_to_free);
	send_response_status(socket_cliente, SUCCESS);
}

void respond_cpy(muse_body* body, char* id, int socket_cliente) {	
	uint32_t dst;
	memcpy(&dst, body->content, sizeof(uint32_t));	
	int size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(int));
	memcpy(MEMORY, body->content + sizeof(uint32_t) + sizeof(int), size);

	// log_info(logger, "El cliente con id: %s hizo cpy a dst: %u, %i bytes", id, dst, size);

	send_response_status(socket_cliente, SUCCESS);
}

void respond_map(muse_body* body, char* id, int socket_cliente) {
	int path_size;
	memcpy(&path_size, body->content, sizeof(int));
	char* path = malloc(path_size * sizeof(char));
	memcpy(path, body->content + sizeof(int), path_size);
	
	size_t length;
	memcpy(&length, body->content + sizeof(int) + path_size, sizeof(size_t));
	map_flag flags;
	memcpy(&flags, body->content + sizeof(int) + path_size + sizeof(size_t), sizeof(int));

	// log_info(logger, "El cliente con id: %s hizo map a: %s, de %i bytes, flag: %i", id, path, length, flags);

	return;
}

void respond_sync(muse_body* body, char* id, int socket_cliente) {
	uint32_t addr;
	memcpy(&addr, body->content, sizeof(uint32_t));
	size_t len;
	memcpy(&len, body->content + sizeof(uint32_t), sizeof(size_t));

	// log_info(logger, "El cliente con id: %s hizo sync a la addr: %u de %i bytes", id, addr, len);

	return;
}

void respond_unmap(muse_body* body, char* id, int socket_cliente) {
	uint32_t addr;
	memcpy(&addr, body->content, sizeof(uint32_t));

	// log_info(logger, "El cliente con id: %s hizo unmap a la addr: %u", id, addr);

	return;
}

int respond_to_client(int cliente_fd) {
	while(1) {
		int cod_op = recv_muse_op_code(cliente_fd);
		char* id = recv_muse_id(cliente_fd);
		muse_body* body = recv_body(cliente_fd);
		switch(cod_op) {
			case INIT_MUSE:
				respond_init(body, id, cliente_fd);
				break;
			case ALLOC:
				// log_info(logger, "muse_alloc.");
				respond_alloc(body, id, cliente_fd);
				break;
			case FREE:
				// log_info(logger, "muse_free.");
				respond_free(body, id, cliente_fd);
				break;
			case GET:
				// log_info(logger, "muse_get.");
				respond_get(body, id, cliente_fd);
				break;
			case CPY:
				// log_info(logger, "muse_copy.");
				respond_cpy(body, id, cliente_fd);
				break;
			case MAP:
				// log_info(logger, "muse_map.");
				respond_map(body, id, cliente_fd);
				break;
			case SYNC:
				// log_info(logger, "muse_sync.");
				respond_sync(body, id, cliente_fd);
				break;
			case UNMAP:
				// log_info(logger, "muse_unmap.");
				respond_unmap(body, id, cliente_fd);
				break;
			case DISCONNECT_MUSE:
				// log_info(logger, "El cliente se desconecto.");
				return EXIT_FAILURE;
				break;
			default:
				// log_warning(logger, "Operacion desconocida. No quieras meter la pata");
				break;
		}
	}
}

int main(void) {
	logger = log_create("./logs/MUSE.log", "Servidor", 1, LOG_LEVEL_DEBUG);
	config = config_create("./program.config");

	IP = "127.0.0.1";
	PORT = config_get_string_value(config, "LISTEN_PORT");
	MEMORY_SIZE = config_get_int_value(config, "MEMORY_SIZE");
	PAGE_SIZE = config_get_int_value(config, "PAGE_SIZE");
	SWAP_SIZE = config_get_int_value(config, "SWAP_SIZE");

	log_info(logger, "Port: %s", PORT);
	log_info(logger, "Memory size: %i", MEMORY_SIZE);
	log_info(logger, "Page size: %i", PAGE_SIZE);
	log_info(logger, "Number of pages: %i", MEMORY_SIZE / PAGE_SIZE);

	int server_fd = init_server(IP, PORT);
	
	if (server_fd == -1) {
		return EXIT_FAILURE;
	}

	init_structures(MEMORY_SIZE, PAGE_SIZE);

	pthread_t hilo;
	int r1;
	while (1) {
		int cliente_fd = recibir_cliente(server_fd);
	    r1 = pthread_create(&hilo, NULL, (void*) respond_to_client, (void*) cliente_fd);
	}
	return EXIT_SUCCESS;
}
