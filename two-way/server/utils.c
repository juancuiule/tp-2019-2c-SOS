#include "utils.h"

// Cliente
// - crear_conexion
// - liberar_conexion

int crear_conexion(char *IP, char* PORT) {
	struct addrinfo hints, *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(IP, PORT, &hints, &server_info);

	int socket_cliente = socket(server_info->ai_family, server_info->ai_socktype, server_info->ai_protocol);
	int connect_result = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);

	if(connect_result == -1) {
		log_error(logger, "Error al conectar");
		return -1;
	}

	freeaddrinfo(server_info);

	return socket_cliente;
}

void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}

// Server
// - iniciar_servidor
// - recibir_cliente
// - recibir_operacion
// - recibir_buffer

int iniciar_servidor(char* IP, char* PORT) {
	int socket_servidor;

    struct addrinfo hints, *servinfo, *p;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = PF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(IP, PORT, &hints, &servinfo);

    for (p = servinfo; p != NULL; p = p->ai_next) {
		socket_servidor = socket(
			p->ai_family,
			p->ai_socktype,
			p->ai_protocol
		);

        if (socket_servidor == -1) {
            continue;
		}

		int bind_result = bind(
			socket_servidor,
			p->ai_addr,
			p->ai_addrlen
		);

        if (bind_result == -1) {
            close(socket_servidor);
			freeaddrinfo(servinfo);
			log_error(logger, "Fallo el bind.");
            return -1;
        }

        break;
    }

	listen(socket_servidor, SOMAXCONN);

	log_info(logger, "Servidor listo para recibir clientes");

    freeaddrinfo(servinfo);

    return socket_servidor;
}

int recibir_cliente(int socket_servidor) {
	struct sockaddr_in dir_cliente;
	int tam_direccion = sizeof(struct sockaddr_in);

	int socket_cliente = accept(
		socket_servidor,
		(void*) &dir_cliente,
		&tam_direccion
	);

	log_info(logger, "Se conecto un cliente! %i", socket_cliente);

	return socket_cliente;
}

int recibir_operacion(int socket_cliente) {
	int cod_op;

	int recv_result = recv(
		socket_cliente,
		&cod_op,
		sizeof(int),
		MSG_WAITALL
	);

	if (recv_result != 0) {
		log_info(logger, "cod_op: %i", cod_op);
		return cod_op;
	} else {
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente) {
	void * buffer;

	// recibir tamaño del buffer y ponerlo en "size"
	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);

	buffer = malloc(*size);

	// recibir buffer
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

// Cliente - Servidor - Paquetes/Serializacion

t_paquete* crear_paquete(op_code codigo_operacion, int size, void* buffer) {
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = codigo_operacion;
	crear_buffer(paquete, size, buffer);
	return paquete;
}

void eliminar_paquete(t_paquete* paquete) {
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void crear_buffer(t_paquete* paquete, int size, void* buffer) {
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = size;
	paquete->buffer->stream = malloc(size);
	memcpy(paquete->buffer->stream, buffer, size);
}

void* serializar_paquete(t_paquete* paquete, int bytes) {
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento += sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento += paquete->buffer->size;

	return magic;
}

void enviar_paquete(t_paquete* paquete, int socket_cliente) {
	int bytes = paquete->buffer->size + 2 * sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	free(a_enviar);
}

void enviar_mensaje(char* mensaje, int socket_cliente) {
	t_paquete* paquete = crear_paquete(
		MENSAJE,
		strlen(mensaje)	+ 1,
		mensaje
	);
	enviar_paquete(paquete, socket_cliente);
	eliminar_paquete(paquete);
}