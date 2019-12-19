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

t_log* logger;

int create_connection(char *IP, int INT_PORT);
void free_connection(int socket_cliente);
int init_server(char* IP, char* PORT);
uint32_t recv_uint(int socket_cliente);
void* recv_int(int socket_cliente);
void* recv_buffer(int* size, int socket_cliente);
int recibir_cliente(int socket_servidor);

#endif
