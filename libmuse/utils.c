#include "utils.h"
#include "network.h"

muse_package* create_package(muse_header* header, muse_body* body) {
	muse_package* package = malloc(sizeof(muse_package));
	package->header = header;
	package->body = body;
	return package;
}

muse_response* create_response(response_status status, muse_body* body) {
	muse_response* response = malloc(sizeof(muse_response));
	response->status = status;
	response->body = body;
	return response;
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
	header->id->pid = getpid();
	return header;
}

muse_body* create_body() {
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
		size // size of the value to add
	);
	memcpy(body->content + body->content_size, &size, sizeof(int));
	memcpy(body->content + body->content_size + sizeof(int), value, size);
	body->content_size += size + sizeof(int);
}

void add_fixed_to_body(muse_body* body, int size, void* value) {
	body->content = realloc(
		body->content, // *ptr
		body->content_size + // prev size
		size // size of the value to add
	);
	memcpy(body->content + body->content_size, &value, size);
	body->content_size += size;
}

muse_body* recv_body(int socket) {
	muse_body* body = malloc(sizeof(muse_body));
	body->content_size = recv_int(socket);
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

void free_response(muse_response* response) {
	free(response->body->content);
	free(response->body);
	free(response);
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

void send_muse_op_code(int socket_cliente, muse_op_code op_code) {
	muse_header* header = create_header(op_code);
	muse_body* body = create_body();
	muse_package* package = create_package(header, body);
	send_package(package, socket_cliente);
	free_package(package);
}

void send_response_status(int socket_cliente, response_status status) {
	muse_body* body = create_body();
	muse_response* response = create_response(status, body);
	send_response(response, socket_cliente);
	free_response(response);
}

muse_op_code recv_muse_op_code(int socket_cliente) {
	return recv_enum(socket_cliente);
}

response_status recv_response_status(int socket_cliente) {
	return recv_enum(socket_cliente);
}

int recv_enum(int socket_cliente) {
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
	int pid = recv_int(socket_cliente);
	int ip_size;

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
