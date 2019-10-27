#include "libmuse.h"

#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

void enviar_algo(char* algo, int conexion) {
	enviar_mensaje(algo, conexion);

	int cod_op;
	int size;

	// recibir codigo de operacion, un entero que tenemos como enum
	int recv_result = recv(
		conexion,
		&cod_op,
		sizeof(int),
		MSG_WAITALL
	);

	if (recv_result != 0) {
		log_info(logger, "cod_op: %i", cod_op);
	} else {
		close(conexion);
	}

	void * buffer;

	// recibir tamaño del buffer y ponerlo en "size"
	recv(conexion, &size, sizeof(int), MSG_WAITALL);
	log_info(logger, "size: %i", size);

	buffer = malloc(size);

	// recibir buffer
	recv(conexion, buffer, size, MSG_WAITALL);
	log_info(logger, "buffer: %s", buffer);
}

int conexion;

int muse_init(int id, char* ip, int puerto){
	logger = log_create("./logs/libMuse.log", "Cliente", 1, LOG_LEVEL_INFO);


	conexion = crear_conexion(ip, puerto);

	log_info(logger, "Conectandome a %s:%s", ip, puerto);

	return 0;
}

void muse_close(){
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}

uint32_t muse_alloc(uint32_t tam){
	// TODO: cambiar enviar_algo por una función que serialice alloc
	enviar_algo("muse_alloc", conexion);
	return 0;
    // return (uint32_t) malloc(tam);
}

void muse_free(uint32_t dir) {
    free((void*) dir);
}

int muse_get(void* dst, uint32_t src, size_t n){
    memcpy(dst, (void*) src, n);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n){
    memcpy((void*) dst, src, n);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////
uint32_t muse_map(char *path, size_t length, int flags){
    return 0;
}

int muse_sync(uint32_t addr, size_t len){
    return 0;
}

int muse_unmap(uint32_t dir){
    return 0;
}
