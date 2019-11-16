#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "libSUSE.h"
#include "utils.h"

int max_tid = 0;

hilo_t* crear_nuevo_hilo(int tid, int pid) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->tid = tid;
	hilo->pid = pid;
	hilo->tiempo_ejecucion = 0;
	hilo->tiempo_espera = 0;
	hilo->tiempo_cpu = 0;
	hilo->estimacion_anterior = 0;
	hilo->rafaga_anterior = 0;
	return hilo;
}

int ejecutar_operacion(int tid, int operacion) {
	int conexion = conectarse_a_suse();
	int pid = getpid();
	t_paquete* paquete = crear_paquete(operacion);
	hilo_t* hilo = crear_nuevo_hilo(tid, pid);
	agregar_a_paquete(paquete, hilo, sizeof(hilo_t));
	enviar_paquete(paquete, conexion);
	return 0;
}

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

int suse_create(int tid){
	if (tid > max_tid) max_tid = tid;
	//printf("suse_create(%i)\n", tid);
	return ejecutar_operacion(tid, 1);
}

int suse_schedule_next(void){
	int next = max_tid;
	//printf("suse_schedule_next() (hilo %i)\n", next);
	return next;
}

int suse_join(int tid){
	//printf("suse_join(%i)\n", tid);
	return ejecutar_operacion(tid, 3);
}

int suse_close(int tid){
	//printf("suse_close(%i)\n", tid);
	return ejecutar_operacion(tid, 4);
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

	int opcode, size, tid;
	memcpy(&opcode, magic, sizeof(int));
	memcpy(&size, magic + 4, sizeof(int));
	memcpy(&tid, magic + 8, sizeof(int));

	return magic;
}

t_paquete* crear_paquete(int operacion)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = operacion;
	crear_buffer(paquete);
	return paquete;
}

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);
	paquete->buffer->size += tamanio + sizeof(int);
	int opcode;
	int size;
	int val;
	memcpy(&opcode, &paquete->codigo_operacion, sizeof(int));
	memcpy(&size, &paquete->buffer->size, sizeof(int));
	memcpy(&val, valor, sizeof(int));
}

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);
	send(socket_cliente, a_enviar, bytes, 0);
	int opcode, size, tid;
	memcpy(&opcode, a_enviar, 4);
	memcpy(&size, a_enviar + 4, 4);
	memcpy(&tid, a_enviar + 8, 4);
	/*
	printf("opcode enviado: %i\n", opcode);
	printf("size enviado: %i\n", size);
	printf("TID enviado: %i\n", tid);
	*/
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
