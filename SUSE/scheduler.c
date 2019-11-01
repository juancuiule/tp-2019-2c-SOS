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
	  int conexion_servidor, conexion_cliente;
	  socklen_t longc;
	  struct sockaddr_in servidor, cliente;
	  char buffer[100];
	  conexion_servidor = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&servidor, sizeof(servidor));
	  servidor.sin_family = AF_INET;
	  servidor.sin_port = htons(LISTEN_PORT);
	  servidor.sin_addr.s_addr = inet_addr(LISTEN_IP);

	  if(bind(conexion_servidor, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
	  {
	    printf("Error al asociar el puerto a la conexion\n");
	    close(conexion_servidor);
	    return;
	  }

	  listen(conexion_servidor, 3);
	  printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
	  longc = sizeof(cliente);
	  conexion_cliente = accept(conexion_servidor, (struct sockaddr *)&cliente, &longc);

	  if(conexion_cliente<0)
	  {
	    printf("Error al aceptar trafico\n");
	    close(conexion_servidor);
	    return;
	  }

	  printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  if(recv(conexion_cliente, buffer, 100, 0) < 0)
	  {
	    printf("Error al recibir los datos\n");
	    close(conexion_servidor);
	    return;
	  }
	  else
	  {
	    printf("%s\n", buffer);
	    bzero((char *)&buffer, sizeof(buffer));
	    send(conexion_cliente, "Recibido\n", 13, 0);
	  }
	  close(conexion_servidor);
	  return;
}










