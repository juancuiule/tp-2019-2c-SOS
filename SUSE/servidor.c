/*
 * server.c
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#include "dispatcher.h"
#include "configuracion.h"
#include "servidor.h"

void inicializar_colas() {
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
}

ult_t* deserializar(void* buffer) {
	ult_t *ult = malloc(1000);
	memcpy(&(ult->pid), buffer, sizeof(ult->pid));
	memcpy(&(ult->tid), buffer + 1, sizeof(ult->tid));
	return ult;
}

int servidor() {
	  int conexion_servidor, conexion_cliente, tid;
	  socklen_t longc;
	  struct sockaddr_in servidor, cliente;
	  ult_t* ult;
	  void* buffer = malloc(100);
	  conexion_servidor = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&servidor, sizeof(servidor));
	  servidor.sin_family = AF_INET;
	  servidor.sin_port = htons(LISTEN_PORT);
	  servidor.sin_addr.s_addr = inet_addr(LISTEN_IP);

	  bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor));

	  listen(conexion_servidor, 3);
	  //printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
	  longc = sizeof(cliente);

	  while(1) {
		  conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);
		  //printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
		  recv(conexion_cliente, buffer, 100, 0);
		  ult = deserializar(buffer);
		  printf("TID: %i PID: %i\n", ult->tid, ult->pid);
		  encolar_ult(ult->tid);
		  pthread_create(&nuevo_ult, NULL, (void*)dispatcher, tid);
	      bzero((char *)&buffer, sizeof(buffer));
	      send(conexion_cliente, "Recibido\n", 13, 0);
	  }

	  close(conexion_servidor);
	  return 0;
}

void encolar_ult(int tid) {
	queue_push(cola_new, tid);
	printf("SUSE: nuevo ULT en NEW (TID: %i)\n", tid);
}

void mostrar_ults_cola(t_queue* cola) {
	int tid = 0;
	t_queue* aux = malloc(sizeof(t_queue));
	aux = cola;
	printf("SUSE: ULTs en cola: ");

	while (aux->elements->head->next != NULL) {
		tid = queue_peek(aux);
		printf("%i - ", tid);
		aux = aux->elements->head->next;
	}

	free(aux);
}










