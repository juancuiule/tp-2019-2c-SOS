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

        if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
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

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) != 0)
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

mensaje_t* recibir_paquete(int socket_cliente)
{
	void* buffer = malloc(sizeof(mensaje_t));
	recv(socket_cliente, buffer, sizeof(buffer), 0);
	mensaje_t* mensaje = malloc(sizeof(mensaje_t));
	mensaje->hilo = malloc(sizeof(hilo_t));
	memcpy(&(mensaje->operacion), buffer, sizeof(operacion_t));
	memcpy(&(mensaje->hilo->tid), buffer + sizeof(operacion_t), sizeof(int));
	memcpy(&(mensaje->hilo->pid), buffer + sizeof(operacion_t)+ sizeof(int), sizeof(int));
	return mensaje;
}











