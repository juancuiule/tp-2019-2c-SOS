#ifndef socket_h
#define socket_h

#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h> /* memset */
#include <unistd.h> /* close */
#include <sys/socket.h>
#include <sys/types.h>
#include "log.h"
#include <arpa/inet.h>//inet_addr

#define BACKLOG 250
#define BUFFER_LIMIT 1452

typedef struct socket{
	int fd;
}socket_t;

int crear_conexion(char* ip, int puerto);
int crear_servidor(int port);
int aceptar_cliente(int server_socket);

size_t recv_bytes(int socket, void *buffer, size_t size);
size_t send_bytes(int socket, void *buffer, size_t size);
void socket_liberar(int socket);

#endif /* socket_h */
