#ifndef COMMON_PROTOCOL_H_
#define COMMON_PROTOCOL_H_

#include <stdlib.h>
#include "log.h"
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "socket.h"

#define TAM_HEADER 9

typedef enum{
    COD_HANDSHAKE,
	COD_READDIR,
	COD_ERROR
}cod_operation;

typedef struct{
    char cod_proceso;
    int cod_operacion;
    int tam_payload;
}__attribute__((packed))
header_t;

typedef struct{
	header_t header;
    void * payload;
}package_t;

typedef struct{
    int socket;
}pthread_data;

void handshake_recibir(int socket);
void handshake_enviar(int socket, char cod_proc);

package_t paquete_recibir(int socket);
bool paquete_enviar(int socket, package_t paquete);

package_t slz_cod_readdir(const char *path);
void dslz_cod_readdir(void *buffer, char** path);

#endif /* COMMON_PROTOCOL_H_ */
