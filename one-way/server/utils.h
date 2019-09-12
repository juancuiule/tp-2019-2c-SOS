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

typedef enum {
	DESCONECTAR = -1,
	MENSAJE,
	PAQUETE,
} op_code;

t_log* logger;
t_config* config;

int iniciar_servidor(char* IP, char* PORT);
int esperar_cliente(int);

int respond_to_client(int);

char* recibir_buffer(int);
int recibir_operacion(int);

#endif
