#include "MUSE.h"

void respond_init(muse_body* body, char* id, int socket_cliente) {
	create_process_table(id);
}

//void log_bitarray() {
//	log_info(logger, "Used:");
//	log_info(logger, "0 -> %i", bitarray_test_bit(frame_usage_bitmap, 0));
//	log_info(logger, "2 -> %i", bitarray_test_bit(frame_usage_bitmap, 2));
//	log_info(logger, "4 -> %i", bitarray_test_bit(frame_usage_bitmap, 4));
//	log_info(logger, "6 -> %i", bitarray_test_bit(frame_usage_bitmap, 6));
//	log_info(logger, "Not used:");
//	log_info(logger, "1 -> %i", bitarray_test_bit(frame_usage_bitmap, 1));
//	log_info(logger, "3 -> %i", bitarray_test_bit(frame_usage_bitmap, 3));
//	log_info(logger, "5 -> %i", bitarray_test_bit(frame_usage_bitmap, 5));
//	log_info(logger, "7 -> %i", bitarray_test_bit(frame_usage_bitmap, 7));
//}

void respond_alloc(muse_body* body, char* id, int socket_cliente) {
	uint32_t tam_pedido;
	memcpy(&tam_pedido, body->content, sizeof(uint32_t));

	// min amount of frames to ask for the segment
	int frames_to_ask;

	log_info(logger, "El cliente con id: %s hizo muse_malloc con %u", id, tam_pedido);

	process_table* table = get_table_for_process(id);
	process_segment *segment;

	muse_body* r_body = create_body();
	muse_response* response;
	void* dir;

	if (table != NULL) {
		log_info(logger, "El cliente con id: %s tiene tabla de proceso con %i segmentos", id, table->number_of_segments);
		if (table->number_of_segments == 0) {
			frames_to_ask = ceil((double) (tam_pedido + sizeof(bool) + sizeof(uint32_t)) / PAGE_SIZE);

			log_debug(logger, "No hay ningún segmento para el proceso: %s", id);
			log_debug(logger, "Se crea uno con base 0 y %i páginas para hacer un alloc de %i", frames_to_ask, tam_pedido);

			int new_base = last_position(id);
			segment = create_segment(HEAP, new_base);

			for (int var = 0; var < frames_to_ask; var++) {
				int frame_number = find_free_frame(frame_usage_bitmap);
				t_page *new_page = create_page(frame_number);
				add_page_to_segment(segment, new_page);
			}
			int free_dir = find_free_dir(segment, tam_pedido);
			dir = alloc_in_segment(segment, free_dir, tam_pedido);
			add_process_segment(id, segment);
		} else {
			log_debug(logger, "Hay segmentos para el proceso: %s", id);
			log_debug(logger, "Se busca uno para hacer un alloc de %i", tam_pedido);

			segment = find_segment_with_space(table, tam_pedido);

			if (segment != NULL) {
				log_debug(logger, "Hay uno con espacio, la base es: %i, size es: %i", segment->base, segment->size);
				int free_dir = find_free_dir(segment, tam_pedido);
				log_debug(logger, "free dir: %i", free_dir);
				dir = alloc_in_segment(segment, free_dir, tam_pedido);
			} else {
				log_debug(logger, "No hay uno con espacio");

				log_debug(logger, "Busco si hay alguno extensible");
				segment = find_extensible_heap_segment(table);

				if (segment != NULL) {
					log_debug(logger, "Se puede extender el que tiene como base: %i, y size: %i", segment->base, segment->size);

					int free_space = free_space_at_the_end(segment);
					frames_to_ask = ceil((double) (tam_pedido - free_space) / PAGE_SIZE);

					for (int var = 0; var < frames_to_ask; var++) {
						int frame_number = find_free_frame(frame_usage_bitmap);
						log_info(logger, "free frame number = %i", frame_number);
						t_page *new_page = create_page(frame_number);
						add_page_to_segment(segment, new_page);
					}
					int free_dir = find_free_dir(segment, tam_pedido);
					dir = alloc_in_segment(segment, free_dir, tam_pedido);
				} else {
					frames_to_ask = ceil((double) (tam_pedido + sizeof(bool) + sizeof(uint32_t)) / PAGE_SIZE);
					log_debug(logger, "No se puede extender ninguno");
					int new_base = last_position(id);
					log_debug(logger, "Se crea un nuevo segmento desde la base: %i", new_base);
					segment = create_segment(HEAP, new_base);
					for (int var = 0; var < frames_to_ask; var++) {
						int frame_number = find_free_frame(frame_usage_bitmap);
						t_page *new_page = create_page(frame_number);
						add_page_to_segment(segment, new_page);
					}
					int free_dir = find_free_dir(segment, tam_pedido);
					dir = alloc_in_segment(segment, free_dir, tam_pedido);
					add_process_segment(id, segment);
				}
			}
		}
		add_fixed_to_body(r_body, sizeof(uint32_t), segment->base + dir);
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
	void* val = malloc(size);

	log_info(logger, "El cliente con id: %s hizo get a la src: %u de %i bytes", id, src, size);
	process_table* t = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;

	if (t != NULL) {
		if (t->number_of_segments == 0) {
			log_info(logger, "El proceso %s no tiene segmentos, es un seg fault?");
			response = create_response(ERROR, r_body);
		} else {
			process_segment *segment = segment_by_dir(t, src);
			log_info(logger, "segmento->base %i", segment->base);
			int dir_de_pagina = src - segment->base;
			log_info(logger, "dir de pagina %i", dir_de_pagina);

			memcpy(val, segment->pages + dir_de_pagina, size);
			add_to_body(r_body, size, val);
			response = create_response(SUCCESS, r_body);
		}
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		response = create_response(ERROR, r_body);
	}
	send_response(response, socket_cliente);
	free(val);
}

void respond_free(muse_body* body, char* id, int socket_cliente) {
	uint32_t dir_to_free;
	memcpy(&dir_to_free, body->content, sizeof(uint32_t));

	 log_info(logger, "El cliente con id: %s hizo free a la dir: %u", id, dir_to_free);

	// free(dir_to_free);
	send_response_status(socket_cliente, SUCCESS);
}

void respond_cpy(muse_body* body, char* id, int socket_cliente) {	
	uint32_t dst;
	memcpy(&dst, body->content, sizeof(uint32_t));	
	int size;
	memcpy(&size, body->content + sizeof(uint32_t), sizeof(int));
	void* val = malloc(size);
	memcpy(val, body->content + sizeof(uint32_t) + sizeof(int), size);

	log_info(logger, "El cliente con id: %s hizo cpy a dst: %u, %i bytes", id, dst, size);
	process_table* t = get_table_for_process(id);

	if (t != NULL) {
		process_segment *segment;
		if (t->number_of_segments == 0) {
			send_response_status(socket_cliente, ERROR);
		} else {
			segment = segment_by_dir(t, dst);
			log_info(logger, "segmento->base %i", segment->base);
			int dir_de_pagina = dst - segment->base;
			log_info(logger, "dir de pagina %i", dir_de_pagina);

			memcpy(segment->pages + dir_de_pagina, val, size);
			send_response_status(socket_cliente, SUCCESS);
		}
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		send_response_status(socket_cliente, ERROR);
	}
	free(val);
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

	log_info(logger, "El cliente con id: %s hizo map a: %s, de %i bytes, flag: %i", id, path, length, flags);

	process_table* table = get_table_for_process(id);
	muse_body* r_body = create_body();
	muse_response* response;


	void* dir;
	// min amount of frames to ask for the segment
	int frames_to_ask = ceil((double) (length + sizeof(bool) + sizeof(uint32_t)) / PAGE_SIZE);
	process_segment *segment;
	if (table != NULL) {
		log_debug(logger, "Se crea un segmento para hacer un map de %i", length);
		int new_base = last_position(id);
		segment = create_segment(MMAP, new_base);
		for (int var = 0; var < frames_to_ask; var++) {
			int frame_number = find_free_frame(frame_usage_bitmap);
			t_page *new_page = create_page(frame_number);
			add_page_to_segment(segment, new_page);
		}
		int free_dir = find_free_dir(segment, length);
		dir = alloc_in_segment(segment, free_dir, length);
		add_process_segment(id, segment);
		log_info(logger, "ASDFASDFASDFASFASDFA %i", segment->base + dir);
		add_fixed_to_body(r_body, sizeof(uint32_t), segment->base + dir);
		response = create_response(SUCCESS, r_body);
		send_response(response, socket_cliente);
	} else {
		log_info(logger, "El proceso %s no tiene tabla, falto hacer init?");
		send_response_status(socket_cliente, ERROR);
	}
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
//		log_bitarray();
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

	log_debug(logger, "Port: %s", PORT);
	log_debug(logger, "Memory size: %i", MEMORY_SIZE);
	log_debug(logger, "Page size: %i", PAGE_SIZE);
	log_debug(logger, "Number of pages: %i", MEMORY_SIZE / PAGE_SIZE);

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
