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
#include <readline/readline.h>

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

int crear_conexion(char* ip, char* PORT);
void liberar_conexion(int socket_cliente);

void* serializar_paquete(t_paquete* paquete, int bytes);

void enviar_mensaje(char* mensaje, int socket_cliente);
void enviar_paquete(t_paquete* paquete, int socket_cliente);

void eliminar_paquete(t_paquete* paquete);

void leer_consola(void(*accion)(char*));
t_paquete* armar_paquete();
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
t_paquete* crear_paquete(void);
void crear_buffer(t_paquete* paquete);

#endif