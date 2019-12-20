#include "libmuse.h"

#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "utils.h"
#include "network.h"
#include "stdio.h"

int conexion;

int muse_init(int id, char* ip, int puerto) {
	conexion = create_connection(ip, puerto);

	if (conexion != -1) {
		send_muse_op_code(conexion, INIT_MUSE);
		int status = recv_response_status(conexion);
		muse_body* response_body = recv_body(conexion);
		if (status == SUCCESS) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return -1;
	}
}

void muse_close() {
	send_muse_op_code(conexion, DISCONNECT_MUSE);
	if (config != NULL) {
		config_destroy(config);
	}
	free_connection(conexion);
}

uint32_t muse_alloc(uint32_t tam) {
	muse_header* header = create_header(ALLOC);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), tam);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	uint32_t dir;
	memcpy(&dir, response_body->content, sizeof(uint32_t));

	return dir;
}

void muse_free(uint32_t dir) {
	muse_header* header = create_header(FREE);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);	

	return;
}

int muse_get(void* dst, uint32_t src, size_t n) {
	muse_header* header = create_header(GET);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), src);
	add_fixed_to_body(body, sizeof(size_t), n);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);

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
	add_fixed_to_body(body, sizeof(uint32_t), dst);
	add_to_body(body, n, value);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	// chequear resultado...

	free(value);
    return 0;
}

uint32_t muse_map(char *path, size_t length, int flags) {
	muse_header* header = create_header(MAP);
	muse_body* body = create_body();
	add_to_body(body, strlen(path) + 1, path);
	add_fixed_to_body(body, sizeof(size_t), length);
	add_fixed_to_body(body, sizeof(int), flags);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	uint32_t dir;
	memcpy(&dir, response_body->content, sizeof(uint32_t));

	return dir;
}

int muse_sync(uint32_t addr, size_t len){
	muse_header* header = create_header(SYNC);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), addr);
	add_fixed_to_body(body, sizeof(size_t), len);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

    return 0;
}

int muse_unmap(uint32_t dir){
	muse_header* header = create_header(UNMAP);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

    return 0;
}
