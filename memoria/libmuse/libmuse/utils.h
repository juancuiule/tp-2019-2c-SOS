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

typedef enum {
	SUCCESS,
	ERROR,
	SEGFAULT
} response_status;

typedef struct {
	response_status status;
	muse_body* body;
} muse_response;

t_log* logger;
t_config* config;

typedef enum {
	MAP_PRIVATE,
	MAP_SHARED
} map_flag;

muse_package* create_package(muse_header* header, muse_body* body);
muse_header* create_header(muse_op_code code);
muse_body* create_body();
void add_to_body(muse_body* body, int size, void* value);
void add_fixed_to_body(muse_body* body, int size, void* value);
muse_response* create_response(response_status status, muse_body* body);
response_status recv_response_status(int socket_cliente);
int recv_enum(int socket_cliente);
muse_body* recv_body(int socket);
void free_package(muse_package* package);
void* serialize_package(muse_package* package, int bytes);
void* serialize_response(muse_response* response, int bytes);
void send_package(muse_package* package, int socket_cliente);
void send_response(muse_response* response, int socket_cliente);
void send_something(int socket_cliente, muse_op_code op_code, char* something);
void send_muse_op_code(int socket_cliente, muse_op_code op_code);
muse_op_code recv_muse_op_code(int socket_cliente);
char* recv_muse_id(int socket_cliente);

#endif
