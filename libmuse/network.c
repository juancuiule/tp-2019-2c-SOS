#include "network.h"

int create_connection(char *IP, char* PORT) {
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PORT, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	int connect_result = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	if(connect_result == -1) {
		log_error(logger, "Error al conectar");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void free_connection(int socket_cliente) {
	close(socket_cliente);
}

int init_server(char* IP, char* PORT) {
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PORT, &hints, &servinfo);

    for (p = servinfo; p != NULL; p = p->ai_next) {
		socket_servidor = socket(
			p->ai_family,
			p->ai_socktype,
			p->ai_protocol
		);

        if (socket_servidor == -1) {
            continue;
		}

		int bind_result = bind(
			socket_servidor,
			p->ai_addr,
			p->ai_addrlen
		);

        if (bind_result == -1) {
            close(socket_servidor);
			freeaddrinfo(servinfo);
			log_error(logger, "Fallo el bind.");
            return -1;
        }

        break;
    }

	listen(socket_servidor, SOMAXCONN);

	log_info(logger, "Servidor listo para recibir clientes");

    freeaddrinfo(servinfo);

    return socket_servidor;
}

uint32_t recv_uint(int socket_cliente) {
	uint32_t size;
	recv(socket_cliente, &size, sizeof(uint32_t), MSG_WAITALL);
	return size;
}

void* recv_int(int socket_cliente) {
	int size;
	recv(socket_cliente, &size, sizeof(int), MSG_WAITALL);
	return size;
}

void* recv_buffer(int* size, int socket_cliente) {
	void* buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

	buffer = malloc(*size);

	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

int recibir_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(
		socket_servidor,
		(void*) &dir_cliente,
		&tam_direccion
	);

	log_info(logger, "Se conecto un cliente! %i", socket_cliente);

	return socket_cliente;
}