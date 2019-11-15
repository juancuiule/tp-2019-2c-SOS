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

muse_package* create_package(muse_header* header, muse_body* body) {
	muse_package* package = malloc(sizeof(muse_package));
	package->header = header;
	package->body = body;
	return package;
}

muse_header* create_header(muse_op_code code) {
	char* ip = "127.0.0.1"; // getip acá
	int ip_size = strlen(ip) + 1;

	muse_header* header = malloc(sizeof(muse_header));
	header->code = code;
	header->id = malloc(sizeof(muse_id));
	header->id->ip_size = ip_size;
	header->id->ip = malloc(ip_size);
	memcpy(header->id->ip, ip, ip_size);
	header->id->pid = 1;
	return header;
}

muse_body* create_empty_body() {
	muse_body* body = malloc(sizeof(muse_body));
	body->content_size = 0;
	body->content = NULL;
	return body;
}

void add_to_body(muse_body* body, int size, void* value) {
	body->content = realloc(
		body->content, // *ptr
		body->content_size + // prev size
		sizeof(int) + // new size value indicator
		size // size of the value to ad
	);
	memcpy(body->content + body->content_size, &size, sizeof(int));
	memcpy(body->content + body->content_size + sizeof(int), value, size);
	body->content_size += size + sizeof(int);
}

void add_fixed_to_body(muse_body* body, int size, void* value) {
	body->content = realloc(
		body->content, // *ptr
		body->content_size + // prev size
		size // size of the value to ad
	);
	memcpy(body->content + body->content_size, &value, size);
	body->content_size += size;
}

muse_body* create_body(int content_size, void* content) {
	muse_body* body = create_empty_body();
	add_to_body(body, content_size, content);
	return body;
}

muse_response* create_response(response_status status, muse_body* body) {
	muse_response* response = malloc(sizeof(muse_response));
	response->status = status;
	response->body = body;
	return response;
}

response_status recv_response_status(int socket_cliente) {
	response_status status;

	int recv_result = recv(
		socket_cliente,
		&status,
		sizeof(int),
		MSG_WAITALL
	);

	if (recv_result != 0) {
		log_info(logger, "status: %i", status);
		return status;
	} else {
		close(socket_cliente);
		return -1;
	}
}

muse_body* recv_body(int socket) {
	muse_body* body = malloc(sizeof(muse_body));
	body->content_size = recv_int(socket);
	log_info(logger, "El content size es %i", body->content_size);
	if (body->content_size > 0) {
		body->content = malloc(body->content_size);
		recv(socket, body->content, body->content_size, MSG_WAITALL);
	}
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
	
	return magic;
}

void* serialize_response(muse_response* response, int bytes) {
	void* magic = malloc(bytes);
	int offset = 0;

	memcpy(magic + offset, &(response->status), sizeof(int));
	offset += sizeof(int);

	memcpy(magic + offset, &(response->body->content_size), sizeof(int));
	offset += sizeof(int);
	memcpy(magic + offset, response->body->content, response->body->content_size);
	offset += response->body->content_size;
	
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

void send_response(muse_response* response, int socket_cliente) {
	int bytes = response->body->content_size + 2 * sizeof(int);
	void* to_send = serialize_response(response, bytes);
	send(socket_cliente, to_send, bytes, 0);
	free(to_send);
}

void send_int(int socket_cliente, muse_op_code op_code, uint32_t tam){
	char str[11];
	snprintf(str, sizeof str, "%u", tam);
	send_something(socket_cliente, op_code, str);
}

void send_something(int socket_cliente, muse_op_code op_code, char* something){
	muse_header* header = create_header(op_code);
	muse_body* body = create_body(strlen(something) + 1, something);
	muse_package* package = create_package(header, body);
	send_package(package, socket_cliente);
	free_package(package);
}

void send_code(int socket_cliente, muse_op_code op_code){
	muse_header* header = create_header(op_code);
	muse_body* body = create_empty_body();
	muse_package* package = create_package(header, body);
	send_package(package, socket_cliente);
	free_package(package);
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
		return op_code;
	} else {
		close(socket_cliente);
		return -1;
	}
}

char* recv_muse_id(int socket_cliente) {
	int pid;
	int ip_size;

	recv(
		socket_cliente,
		&pid,
		sizeof(int),
		MSG_WAITALL
	);

	char pid_str[11];
	snprintf(pid_str, sizeof pid_str, "%i", pid);
	int len1 = strlen(pid_str);

	char* separador = "/";
	int len2 = strlen(separador);

	char* ip = recv_buffer(&ip_size, socket_cliente);
	int len3 = strlen(ip);

	char *id = malloc(len1 + len2 + len3 + 1);
	memcpy(id, pid_str, len1);
    memcpy(id + len1, separador, len2);
	memcpy(id + len1 + len2, ip, len3 + 1);
	return id;
}
