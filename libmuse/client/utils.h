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

typedef struct {
	int size;
	void* stream;
} t_buffer;

typedef struct {
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

t_log* logger;
t_config* config;

int crear_conexion(char *IP, char* PORT);
void liberar_conexion(int socket_cliente);

int iniciar_servidor(char* IP, char* PORT);
int recibir_cliente(int socket_servidor);
int recibir_operacion(int socket_cliente);
void* recibir_buffer(int* size, int socket_cliente);

t_paquete* crear_paquete(op_code codigo_operacion, int size, void* buffer);
void eliminar_paquete(t_paquete* paquete);
void crear_buffer(t_paquete* paquete, int size, void* buffer);
void* serializar_paquete(t_paquete* paquete, int bytes);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void enviar_mensaje(char* mensaje, int socket_cliente);

#endif