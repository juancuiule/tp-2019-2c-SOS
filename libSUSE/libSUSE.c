#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "libSUSE.h"
#include "utils.h"

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

int enviar_datos_hilo(int tid, int pid, int conexion, int operacion) {
	/*
	mensaje_t* mensaje = malloc(sizeof(mensaje));
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->pid = pid;
	hilo->tid = tid;
	mensaje->operacion = operacion;
	mensaje->hilo = hilo;
	send(conexion, (void*)mensaje, sizeof(mensaje), 0);
	*/
	//enviar_mensaje();
}

int suse_create(int tid){
	int conexion = conectarse_a_suse();
	t_paquete* paquete = crear_paquete();
	agregar_a_paquete(paquete, tid, sizeof(tid));
	agregar_a_paquete(paquete, getpid(), sizeof(pid_t));
	agregar_a_paquete(paquete, 1, sizeof(int));
	enviar_paquete(paquete, conexion);
	return 0;
}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	//int conexion = conectarse_a_suse();
	//enviar_datos_ult(tid, getpid(), conexion, 3);
	return 0;
}

int suse_close(int tid){
	int conexion = conectarse_a_suse();
	enviar_datos_hilo(tid, getpid(), conexion, 4);
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

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
