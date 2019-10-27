/*
 * libSUSE.c
 *
 *  Created on: 26 oct. 2019
 *      Author: utnso
 */


#include "libSUSE.h"

int suse_create(int tid) {
	  struct sockaddr_in cliente;
	  struct hostent *servidor;
	  servidor = gethostbyname("127.0.0.1");
	  int puerto, conexion;
	  char buffer[100];
	  conexion = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&cliente, sizeof((char *)&cliente));
	  cliente.sin_family = AF_INET;
	  cliente.sin_port = htons(8000);
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
	  connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente));
	  printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  send(conexion, tid, sizeof(tid), 0);
	  return 0;
}

int suse_schedule_next() {
	return 0;
}

int suse_join(int tid) {
	return 0;
}

int suse_close(int tid) {
	return 0;
}

int suse_wait(int tid, char* semaforo) {
	return 0;
}

int suse_signal(int tid, char* semaforo) {
	return 0;
}

void hilolay_init() {
	printf("ejecuta hilolay_init()\n");
	hilolay_operations hilo_ops = {
			.suse_create = &suse_create,
			.suse_schedule_next = &suse_schedule_next,
			.suse_join = &suse_join,
			.suse_close = &suse_close
	};

	init_internal(&hilo_ops);
}
