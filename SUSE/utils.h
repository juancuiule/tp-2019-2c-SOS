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
	SUCCESS,
	ERROR
} response_status;

t_log* logger;
t_config* config;

int create_connection(char *IP, char* PORT);
void free_connection(int socket_cliente);

void send_connect(int socket_cliente);
void send_disconnet(int socket_cliente);
int send_alloc(int socket_cliente, uint32_t tam);
void send_free(int socket_cliente, uint32_t dir);

int init_server(char* IP, char* PORT);
int recibir_cliente(int socket_servidor);
int respond_to_client(int);
void* recv_buffer(int* size, int socket_cliente);

#endif
