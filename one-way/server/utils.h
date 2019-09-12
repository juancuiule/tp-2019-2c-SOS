#ifndef CONEXIONES_H_
#define CONEXIONES_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/config.h>

typedef enum {
	DESCONECTAR = -1,
	MENSAJE,
	PAQUETE,
} op_code;

t_log* logger;
t_config* config;

int iniciar_servidor(char*, char*, t_log*);
int esperar_cliente(int, void(*log)(char*));

int respond_to_client(int, t_log*);

char* recibir_buffer(int);
int recibir_operacion(int);

#endif
