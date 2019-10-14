#include "alumnos.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

/* Lib implementation: It'll only schedule the last thread that was created */
int max_tid = 0;

int conectar_con_suse() {
	  struct sockaddr_in cliente;
	  struct hostent *servidor;
	  servidor = gethostbyname("localhost");

	  if(servidor == NULL)
	  {
	    printf("Host erróneo\n");
	    return 1;
	  }

	  char buffer[100];
	  int conexion = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&cliente, sizeof((char *)&cliente));
	  cliente.sin_family = AF_INET;
	  cliente.sin_port = htons(8000);
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));

	  if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	  {
	    printf("Error conectando con el host\n");
	    close(conexion);
	    return 1;
	  }

	  printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  printf("Escribe un mensaje: ");
	  fgets(buffer, 100, stdin);
	  send(conexion, buffer, 100, 0);
	  bzero(buffer, 100);
	  recv(conexion, buffer, 100, 0);
	  printf("%s", buffer);
	  return 0;
}

int suse_create(int tid){
	if (tid > max_tid) max_tid = tid;
	return 0;
}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	// Not supported
	return 0;
}

int suse_close(int tid){
	printf("Closed thread %i\n", tid);
	max_tid--;
	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close
};

void hilolay_init(void){
	init_internal(&hiloops);
	conectar_con_suse();
}













