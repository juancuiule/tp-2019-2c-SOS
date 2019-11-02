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
	DISCONNECT_MUSE = -1,
	INIT_MUSE,
	ALLOC,
	FREE,
	GET,
	CPY,
	MAP,
	SYNC,
	UNMAP
} muse_op_code;

typedef struct {
	int pid;
	int ip_size;
	char* ip;
} muse_id;

typedef struct {
	muse_op_code code;
	muse_id* id;
} muse_header;

typedef struct {
	int content_size;
	void* content;
} muse_body;

typedef struct {
	muse_header* header;
	muse_body* body;
} muse_package;

t_log* logger;
t_config* config;

int create_connection(char *IP, char* PORT);
void free_connection(int socket_cliente);

muse_package* create_package(muse_header* header, muse_body* body);
muse_header* create_header(muse_op_code code);
muse_body* create_body(int content_size, void* content);
void free_package(muse_package* package);
void* serialize_package(muse_package* package, int bytes);
void send_package(muse_package* package, int socket_cliente);
void send_something(int socket_cliente, muse_op_code op_code, char* something);
void send_connect(int socket_cliente);
void send_disconnet(int socket_cliente);
int send_alloc(int socket_cliente, uint32_t tam);
void send_free(int socket_cliente, uint32_t dir);

int init_server(char* IP, char* PORT);
int recibir_cliente(int socket_servidor);
muse_op_code recv_muse_op_code(int socket_cliente);
char* recv_muse_id(int socket_cliente);
void* recv_buffer(int* size, int socket_cliente);

#endif
