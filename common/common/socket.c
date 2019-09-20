#include "socket.h"

int crear_conexion(char *ip, int puerto)
{
	int sock;
	struct sockaddr_in dir_server;

    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
    	perror("socket");
    	return -1;
    }

    dir_server.sin_family = AF_INET;
    dir_server.sin_addr.s_addr = inet_addr(ip);
    dir_server.sin_port = htons(puerto);

    if (connect(sock , (struct sockaddr *)&dir_server , sizeof(dir_server)) < 0)
    {
        perror("No se pudo conectar");
        return -1;
    }
	return sock;
}

int crear_servidor(int puerto)
{
	int fd_socket;
	struct sockaddr_in my_addr;

	int yes=1;
	if ((fd_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("socket");
	}
	if (setsockopt(fd_socket,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1)
	{
		perror("setsockopt");
	}

	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(puerto);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	memset(&(my_addr.sin_zero), '\0', 8);

	if (bind(fd_socket, (struct sockaddr *)&my_addr, sizeof(struct sockaddr)) == -1)
	{
		perror("bind");
	}

	log_msje_info("Escuchando nuevas conexiones");
	if (listen(fd_socket,BACKLOG) == -1)
	{
		perror("listen");
	}

	return fd_socket;
}

int aceptar_cliente(int socket_servidor)
{
	struct sockaddr cliente;
	socklen_t longitudCliente = sizeof(cliente);
	int new_socket;

	new_socket = accept(socket_servidor, &cliente, &longitudCliente);

	return new_socket;
}

size_t recv_bytes(int socket, void *buffer, size_t size)
{
	size_t bytes_recibidos = 0, bytes;

	while(bytes_recibidos < size)
	{
		bytes = size - bytes_recibidos < BUFFER_LIMIT ? size - bytes_recibidos : BUFFER_LIMIT;
		ssize_t c = recv(socket, buffer + bytes_recibidos, bytes, 0);

		if(c == -1)
		{
			log_msje_error("Error al recibir bytes del socket [ %d ]", socket);
			return c;
		}
		if(c == 0)
		{
			log_msje_error("Se cerro conexion con el socket [ %d ]", socket);
			return c;
		}

		bytes_recibidos+=c;
	}

	return bytes_recibidos;
}

size_t send_bytes(int socket, void *buffer, size_t size)
{
	size_t bytes_enviados = 0, bytes;

	while(bytes_enviados < size)
	{
		bytes = size - bytes_enviados < BUFFER_LIMIT ? size - bytes_enviados : BUFFER_LIMIT;
		ssize_t c = send(socket, buffer + bytes_enviados, bytes, 0);

		if(c == -1)
		{
			log_msje_error("Error al enviar bytes por socket [ %d ]", socket);
			return c;
		}
		if(c == 0)
		{
			log_msje_error("Se cerro conexion con el socket [ %d ]", socket);
			return c;
		}

		bytes_enviados+=c;
	}

	log_msje_info("Exito enviando bytes send bytes");

	return bytes_enviados;
}

void socket_liberar(int socket)
{
	int res;
	res = close(socket);
	if(res!=-1)
		log_msje_info("socket [ %d ] cerrado", socket);
}
