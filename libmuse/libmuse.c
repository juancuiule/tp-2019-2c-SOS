#include "libmuse.h"

#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int conexion;

int muse_init(int id, char* ip, int puerto){
	logger = log_create("./logs/libmuse.log", "libmuse", 1, LOG_LEVEL_INFO);

	log_info(logger, "Conectandome a %s:%s", ip, puerto);

	conexion = create_connection(ip, puerto);

	if (conexion != -1) {
		send_code(conexion, INIT_MUSE);
		return 0;
	} else {
		return -1;
	}
}

void muse_close(){
	log_info(logger, "muse_close");

	log_destroy(logger);
	config_destroy(config);
	send_code(conexion, DISCONNECT_MUSE);

	free_connection(conexion);
}

uint32_t muse_alloc(uint32_t tam){
	log_info(logger, "muse_alloc: tam = %i", tam);
	
	muse_header* header = create_header(ALLOC);
	muse_body* body = create_empty_body();
	add_fixed_to_body(body, sizeof(uint32_t), tam);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);

	uint32_t dir;
	memcpy(&dir, response_body->content, sizeof(uint32_t));

	log_info(logger, "dir: %u", dir);
	return dir;
}

void muse_free(uint32_t dir) {
	log_info(logger, "muse_free a: %u", dir);
	
	muse_header* header = create_header(FREE);
	muse_body* body = create_empty_body();
	add_fixed_to_body(body, sizeof(uint32_t), dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion); // check status?

	return;
	
}

int muse_get(void* dst, uint32_t src, size_t n){
	log_info(logger, "muse_get a: %u, de %i bytes", src, n);

	muse_header* header = create_header(GET);
	muse_body* body = create_empty_body();
	add_fixed_to_body(body, sizeof(uint32_t), src);
	add_fixed_to_body(body, sizeof(size_t), n);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);

	muse_body* response_body = recv_body(conexion);

	size_t r_size;
	memcpy(&r_size, response_body->content, sizeof(size_t));

	log_info(logger, "Esperaba %i bytes y llegaron %i", n, r_size);

	memcpy(dst, response_body->content + sizeof(size_t), r_size);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n){
    memcpy((void*) dst, src, n);
    return 0;
}

uint32_t muse_map(char *path, size_t length, int flags){
    return 0;
}

int muse_sync(uint32_t addr, size_t len){
    return 0;
}

int muse_unmap(uint32_t dir){
    return 0;
}
