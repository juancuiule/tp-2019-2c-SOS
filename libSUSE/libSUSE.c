#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "libSUSE.h"

int max_tid = 0;

int conectarse_a_suse() {
	struct sockaddr_in cliente;
	struct hostent *servidor;
	servidor = gethostbyname("127.0.0.1");

	if(servidor == NULL)
	{
	  printf("Host erróneo\n");
	  return 1;
	}

	char buffer[100];
	int conexion = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *)&cliente, sizeof((char *)&cliente));

	cliente.sin_family = AF_INET;
	cliente.sin_port = htons(8524);
	bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));

	if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	{
	  printf("Error conectando con el host\n");
	  close(conexion);
	  return 1;
	}

	return conexion;
}

int enviar_datos_ult(int tid, int pid, int conexion) {
	mensaje_t* mensaje = malloc(sizeof(mensaje));
	ult_t* ult = malloc(sizeof(ult_t));
	ult->pid = pid;
	ult->tid = tid;
	mensaje->operacion = 1;
	mensaje->ult = ult;
	send(conexion, (void*)mensaje, sizeof(mensaje), 0);
}

int suse_create(int tid){
	int conexion = conectarse_a_suse();
	enviar_datos_ult(tid, getpid(), conexion);
	return 0;
}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	return 0;
}

int suse_close(int tid){
	int conexion = conectarse_a_suse();

	printf("Closed thread %i\n", tid);
	max_tid--;
	return 0;
}

int suse_wait(int tid, char *sem_name){
	return 0;
}

int suse_signal(int tid, char *sem_name){
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
}
