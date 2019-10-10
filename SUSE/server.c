/*
 * server.c
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#include "server.h"
#include "configuracion.h"

int server() {
	 int server_fd, cliente_fd;
	 struct sockaddr_in servidor, cliente;
	 char buffer[100];
	 server_fd = socket(AF_INET, SOCK_STREAM, 0);
	 bzero((char *)&servidor, sizeof(servidor));
	 servidor.sin_family = AF_INET;
	 servidor.sin_port = htons(LISTEN_PORT);
     servidor.sin_addr.s_addr = INADDR_ANY;

	  if(bind(server_fd, (struct sockaddr *)&servidor, sizeof(servidor)) < 0)
	  {
	    printf("Error al asociar el puerto a la conexion\n");
	    close(server_fd);
	    return 1;
	  }

	  listen(server_fd, 3);
	  printf("A la escucha en el puerto %d\n", ntohs(servidor.sin_port));
	  socklen_t longc = sizeof(cliente);
	  cliente_fd = accept(server_fd, (struct sockaddr *)&cliente, &longc);

	  if(cliente_fd < 0)
	  {
	    printf("Error al aceptar trafico\n");
	    close(server_fd);
	    return 1;
	  }

	  printf("Conectando con %s:%d\n", inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));

	  while(1) {
		  if(recv(cliente_fd, buffer, 100, 0) < 0)
		  {
			  printf("Error al recibir los datos\n");
			  close(server_fd);
			  return 1;
		  }
		  else
		  {
			  printf("%s\n", buffer);
			  bzero((char *)&buffer, sizeof(buffer));
			  send(cliente_fd, "Recibido\n", 13, 0);
		  }
	  }

	  close(server_fd);
	  return 0;
}
