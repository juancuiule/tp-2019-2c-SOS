/*
 * server.c
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#include "scheduler.h"
#include "dispatcher.h"
#include "configuracion.h"

void inicializar_colas() {
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
}

void scheduler() {
	 int conexion_servidor, conexion_cliente, tid;
	 struct sockaddr_in servidor, cliente;
	 pthread_t dispatcher_thread;

	 conexion_servidor = socket(AF_INET, SOCK_STREAM, 0);
	 bzero((char *)&servidor, sizeof(servidor));
	 servidor.sin_family = AF_INET;
	 servidor.sin_port = htons(LISTEN_PORT);
     servidor.sin_addr.s_addr = inet_addr(LISTEN_IP);

	 bind(conexion_servidor, (struct sockaddr*)&servidor, sizeof(servidor));
	 listen(conexion_servidor, 3);
	 printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
	 socklen_t longc = sizeof(cliente);
	 conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);

	 printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));

	 while(1) {
		 recv(conexion_cliente, tid, sizeof(tid), 0);
		 printf("Se ha iniciado el hilo %s\n", tid);
		 pthread_create(dispatcher_thread, NULL, (void*)dispatcher, NULL);
	 }

	 close(conexion_servidor);
}










