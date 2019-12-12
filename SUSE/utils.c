/*
 * utils.c
 *
 *  Created on: 4 nov. 2019
 *      Author: utnso
 */

#include "utils.h"

int iniciar_servidor(void)
{
	int socket_servidor;
    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(LISTEN_IP, LISTEN_PORT, &hints, &servinfo);

    for (p=servinfo; p != NULL; p = p->ai_next)
    {
        if ((socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
            continue;

        int yes = 1;
        setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes));

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
        	printf("Falló el bind\n");
            close(socket_servidor);
            continue;
        }

        break;
    }

	listen(socket_servidor, SOMAXCONN);
    freeaddrinfo(servinfo);
    log_info(logger, "Se ha iniciado el servidor.");
    return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(socket_servidor, (void*) &dir_cliente, &tam_direccion);

	//log_info(logger, "Se conecto un cliente!");
	//printf("socket: %i\n", socket_cliente);
	return socket_cliente;
}

int recibir_cod_op(int socket_cliente)
{
	int cod_op = 0;

	if (recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje %s", buffer);
	free(buffer);
}

t_paquete* recibir_paquete(int socket_cliente)
{
	int size;
	void* buffer = recibir_buffer(&size, socket_cliente);
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->stream = malloc(sizeof(t_paquete));
	memcpy(&(paquete->buffer->size), buffer, sizeof(int));
	memcpy(&(paquete->buffer->stream), buffer + sizeof(int) + sizeof(int), size);
	return paquete;
}











