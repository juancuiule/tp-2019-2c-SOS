#include "libmuse.h"

#include <commons/config.h>
#include <commons/log.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

int conexion;

int muse_init(int id, char* ip, int puerto) {
	logger = log_create("./logs/libmuse.log", "libmuse", 1, LOG_LEVEL_INFO);

	log_info(logger, "Conectandome a %s:%s", ip, puerto);

	conexion = create_connection(ip, puerto);

	if (conexion != -1) {
		send_muse_op_code(conexion, INIT_MUSE);
		return 0;
	} else {
		return -1;
	}
}

void muse_close() {
	log_info(logger, "muse_close");

	log_destroy(logger);
	config_destroy(config);
	send_muse_op_code(conexion, DISCONNECT_MUSE);

	free_connection(conexion);
}

uint32_t muse_alloc(uint32_t tam) {
	log_info(logger, "muse_alloc: tam = %i", tam);
	
	muse_header* header = create_header(ALLOC);
	muse_body* body = create_body();
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
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), dir);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion); // check status?
	muse_body* response_body = recv_body(conexion);	

	return;
	
}

int muse_get(void* dst, uint32_t src, size_t n) {
	log_info(logger, "muse_get a: %u, de %i bytes", src, n);

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

	log_info(logger, "Esperaba %i bytes y llegaron %i", n, r_size);

	memcpy(dst, response_body->content + sizeof(size_t), r_size);
    return 0;
}

int muse_cpy(uint32_t dst, void* src, int n) {
	log_info(logger, "muse_cpy a: %u, de %i bytes", dst, n);

	muse_header* header = create_header(CPY);
	muse_body* body = create_body();
	add_fixed_to_body(body, sizeof(uint32_t), dst);
	add_to_body(body, n, src);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

	int status = recv_response_status(conexion);
	muse_body* response_body = recv_body(conexion);
    return 0;
}

/**
* Devuelve un puntero a una posición mappeada de páginas por una cantidad `length` de bytes el archivo del `path` dado.
* @param path Path a un archivo en el FileSystem de MUSE a mappear.
* @param length Cantidad de bytes de memoria a usar para mappear el archivo.
* @param flags
*          MAP_PRIVATE     Solo un proceso/hilo puede mappear el archivo.
*          MAP_SHARED      El segmento asociado al archivo es compartido.
* @return Retorna la posición de memoria de MUSE mappeada.
* @note: Si `length` sobrepasa el tamaño del archivo, toda extensión deberá estar llena de "\0".
* @note: muse_free no libera la memoria mappeada. @see muse_unmap
*/
uint32_t muse_map(char *path, size_t length, int flags) {
	log_info(logger, "muse_map a: %s, de %i bytes, flag: %i", path, length, flags);

	muse_header* header = create_header(MAP);
	muse_body* body = create_body();
	add_to_body(body, strlen(path) + 1, path);
	add_fixed_to_body(body, sizeof(size_t), length);
	add_fixed_to_body(body, sizeof(int), flags);

	muse_package* package = create_package(header, body);
	send_package(package, conexion);

    return 0;
}

/**
* Descarga una cantidad `len` de bytes y lo escribe en el archivo en el FileSystem.
* @param addr Dirección a memoria mappeada.
* @param len Cantidad de bytes a escribir.
* @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
* @note Si `len` es menor que el tamaño de la página en la que se encuentre, se deberá escribir la página completa.
*/
int muse_sync(uint32_t addr, size_t len){
    return 0;
}

/**
* Borra el mappeo a un archivo hecho por muse_map.
* @param dir Dirección a memoria mappeada.
* @param
* @note Esto implicará que todas las futuras utilizaciones de direcciones basadas en `dir` serán accesos inválidos.
* @note Solo se deberá cerrar el archivo mappeado una vez que todos los hilos hayan liberado la misma cantidad de muse_unmap que muse_map.
* @return Si pasa un error, retorna -1. Si la operación se realizó correctamente, retorna 0.
*/
int muse_unmap(uint32_t dir){
    return 0;
}
