#include "libmuse.h"

int conexion;

int muse_init(int id, char* ip, int puerto) {
	printf("id: %i, ip: %s, puerto: %i", id, ip, puerto);
//	conexion = create_connection(ip, puerto);
//
//	if (conexion != -1) {
//		send_muse_op_code(conexion, INIT_MUSE);
//		response_status status = recv_response_status(conexion);
//		recv_body(conexion);
//		// muse_body* response_body = recv_body(conexion);
//
//		if (status == SUCCESS) {
//			return 0;
//		} else {
//			return -1;
//		}
//	} else {
//		return -1;
//	}
	return -1;
}

void muse_close() {
	send_muse_op_code(conexion, DISCONNECT_MUSE);
	log_destroy(logger);
	config_destroy(config);

	free_connection(conexion);
}

uint32_t muse_alloc(uint32_t tam) {
	muse_header* header = create_header(ALLOC);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) tam);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	recv_response_status(conexion);
	//response_status status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	uint32_t dir;
	memcpy(&dir, response_body->content, sizeof(uint32_t));

	return dir;
}

void muse_free(uint32_t dir) {
	muse_header* header = create_header(FREE);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	recv_response_status(conexion);
	//response_status status = recv_response_status(conexion);
	recv_body(conexion);
	// muse_body* response_body = recv_body(conexion);

	return;
}

int muse_get(void* dst, uint32_t src, size_t n) {
	muse_header* header = create_header(GET);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) src);
	add_fixed_to_body(body, sizeof(size_t), (void*) n);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	recv_response_status(conexion);
	// response_status status = recv_response_status(conexion);

	muse_body* response_body = recv_body(conexion);

	size_t r_size;
	memcpy(&r_size, response_body->content, sizeof(size_t));

	memcpy(dst, response_body->content + sizeof(size_t), r_size);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n) {
	void** value = malloc(n);
	memcpy(value, src, n);

	muse_header* header = create_header(CPY);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) dst);
	add_to_body(body, n, value);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	recv_response_status(conexion);
	// response_status status = recv_response_status(conexion);
	recv_body(conexion);
	// muse_body* response_body = recv_body(conexion);

	// chequear resultado...

	free(value);
    return 0;
}

uint32_t muse_map(char *path, size_t length, int flags) {
	muse_header* header = create_header(MAP);
	muse_body* body = create_body();
	add_to_body(body, strlen(path) + 1, path);
	add_fixed_to_body(body, sizeof(size_t), (void*) length);
	add_fixed_to_body(body, sizeof(int), (void*) flags);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	recv_response_status(conexion);
	// response_status status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	uint32_t dir;
	memcpy(&dir, response_body->content, sizeof(uint32_t));

	return dir;
}

int muse_sync(uint32_t addr, size_t len){
	muse_header* header = create_header(SYNC);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) addr);
	add_fixed_to_body(body, sizeof(size_t), (void*) len);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

    return 0;
}

int muse_unmap(uint32_t dir){
	muse_header* header = create_header(UNMAP);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), (void*) dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

    return 0;
}
