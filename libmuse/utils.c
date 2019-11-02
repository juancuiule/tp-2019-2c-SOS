#include "utils.h"

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

muse_package* create_package(muse_header* header, muse_body* body) {
	muse_package* package = malloc(sizeof(muse_package));
	package->header = header;
	package->body = body;
	return package;
}

muse_header* create_header(muse_op_code code) {
	int ip_size = strlen("192.168.0.1") + 1;

	muse_header* header = malloc(sizeof(muse_header));
	header->code = code;
	header->id = malloc(sizeof(muse_id));
	header->id->ip_size = ip_size;
	header->id->ip = malloc(ip_size);
	memcpy(header->id->ip, "192.168.0.1", ip_size);
	header->id->ip = "192.168.0.1";
	header->id->pid = 1;
	return header;
}

muse_body* create_body(int content_size, void* content) {
	muse_body* body = malloc(sizeof(muse_body));
	body->content_size = content_size;
	body->content = malloc(content_size);
	memcpy(body->content, content, content_size);
	return body;
}

void free_package(muse_package* package) {
	free(package->body->content);
	free(package->body);
	free(package);
}

void* serialize_package(muse_package* package, int bytes) {
	void* magic = malloc(bytes);
	int offset = 0;

	memcpy(magic + offset, &(package->header->code), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, &(package->header->id->pid), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, &(package->header->id->ip_size), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, package->header->id->ip, package->header->id->ip_size);
	offset += package->header->id->ip_size;

	memcpy(magic + offset, &(package->body->content_size), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, package->body->content, package->body->content_size);
	offset += package->body->content_size;

	log_info(logger, "offset %i", offset);
	return magic;
}

void send_package(muse_package* package, int socket_cliente) {
	int bytes =
		package->header->id->ip_size +
		package->body->content_size +
		4 * sizeof(int);
	void* to_send = serialize_package(package, bytes);
	send(socket_cliente, to_send, bytes, 0);
	free(to_send);
}

void send_connect(int socket_cliente) {
	muse_header* header = create_header(INIT_MUSE);
	char* message = "Connect";
	muse_body* body = create_body(strlen(message) + 1, message);
	muse_package* package = create_package(header, body);
	send_package(package, socket_cliente);
	free_package(package);
}

void send_disconnet(int socket_cliente) {
	muse_header* header = create_header(DISCONNECT_MUSE);
	char* message = "Disconnect";
	muse_body* body = create_body(strlen(message) + 1, message);
	muse_package* package = create_package(header, body);
	send_package(package, socket_cliente);
	free_package(package);
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

muse_op_code recv_muse_op_code(int socket_cliente) {
	muse_op_code op_code;

	int recv_result = recv(
		socket_cliente,
		&op_code,
		sizeof(int),
		MSG_WAITALL
	);

	if (recv_result != 0) {
		log_info(logger, "op_code: %i", op_code);
		return op_code;
	} else {
		close(socket_cliente);
		return -1;
	}
}

void recv_muse_id(int socket_cliente) {
	int pid;
	int ip_size;

	int recv_result = recv(
		socket_cliente,
		&pid,
		sizeof(int),
		MSG_WAITALL
	);

	char* ip = recv_buffer(&ip_size, socket_cliente);

	log_info(logger, "pid: %i", pid);
	log_info(logger, "ip_size: %i", ip_size);
	log_info(logger, "ip: %s", ip);
}

void* recv_buffer(int* size, int socket_cliente) {
	void* buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

	buffer = malloc(*size);

	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}
