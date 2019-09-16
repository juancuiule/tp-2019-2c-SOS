#ifndef UTILS_H_
#define UTILS_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/collections/list.h>

typedef enum {
	DESCONECTAR = -1,
	MENSAJE,
	PAQUETE,
} op_code;

t_log* logger;
t_config* config;

int iniciar_servidor(char* IP, char* PORT);
int recibir_cliente(int);

int respond_to_client(int);

void* recibir_buffer(int*, int);
void recibir_mensaje(int);
int recibir_operacion(int);

#endif
