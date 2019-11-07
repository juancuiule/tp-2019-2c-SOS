/*
 * libSUSE.h
 *
 *  Created on: 2 nov. 2019
 *      Author: utnso
 */

#ifndef LIBSUSE_H_
#define LIBSUSE_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

typedef struct {
	int tid;
	int pid;
} ult_t;

typedef enum {
	CREATE, // 1
	SCHEDULE_NEXT, // 2
	JOIN, // 3
	CLOSE, // 4
	WAIT, // 5
	SIGNAL // 6
} operacion_t;

typedef struct {
	int operacion;
	ult_t* ult;
} mensaje_t;

void* serializar(ult_t);

#endif /* LIBSUSE_H_ */
