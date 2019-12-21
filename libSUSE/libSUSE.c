#include <hilolay/alumnos.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#include "libSUSE.h"
#include "utils.h"

#define IP "127.0.0.1"
#define PUERTO 8007

int max_tid = 0;
int conexion_con_suse;

int suse_create(int tid){
	int opcode = CREATE;
	int pid = getpid();
	void* buffer = malloc(3 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	memcpy(buffer + 8, &tid, sizeof(int));
	send(conexion_con_suse, buffer, 3 * sizeof(int), MSG_WAITALL);
	return 0;
}

int suse_schedule_next(void){
	int next;
	int opcode = SCHEDULE_NEXT;
	int pid = getpid();
	void* buffer = malloc(2 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	send(conexion_con_suse, buffer, 2 * sizeof(int), MSG_WAITALL);
	recv(conexion_con_suse, &next, sizeof(int), MSG_WAITALL);
	return next;
}

int suse_join(int tid){
	int opcode = JOIN;
	int pid = getpid();
	int tid_hilo_a_bloquear;
	tid_hilo_a_bloquear = hilolay_get_tid();
	void* buffer = malloc(4 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	memcpy(buffer + 8, &tid_hilo_a_bloquear, sizeof(int));
	memcpy(buffer + 12, &tid, sizeof(int));
	send(conexion_con_suse, buffer, 4 * sizeof(int), MSG_WAITALL);
	return 0;
}

int suse_close(int tid){
	int opcode = CLOSE;
	int pid = getpid();
	void* buffer = malloc(3 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	memcpy(buffer + 8, &tid, sizeof(int));
	send(conexion_con_suse, buffer, 3 * sizeof(int), MSG_WAITALL);
	return 0;
}

int suse_wait(int tid, char *sem_name){
	int opcode = WAIT;
	int pid = getpid();
	int tamanio_id = strlen(sem_name);
	void* buffer = malloc(4 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	memcpy(buffer + 8, &tid, sizeof(int));
	memcpy(buffer + 12, &tamanio_id, sizeof(int));
	send(conexion_con_suse, buffer, 4 * sizeof(int), MSG_WAITALL);
	send(conexion_con_suse, sem_name, tamanio_id, MSG_WAITALL);
	return 0;
}

int suse_signal(int tid, char *sem_name){
	int opcode = SIGNAL;
	int pid = getpid();
	int tamanio_id = strlen(sem_name);
	void* buffer = malloc(4 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + 4, &pid, sizeof(int));
	memcpy(buffer + 8, &tid, sizeof(int));
	memcpy(buffer + 12, &tamanio_id, sizeof(int));
	send(conexion_con_suse, buffer, 4 * sizeof(int), MSG_WAITALL);
	send(conexion_con_suse, sem_name, tamanio_id, MSG_WAITALL);
	return 0;
}

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close,
		.suse_wait = &suse_wait,
		.suse_signal = &suse_signal
};

void hilolay_init(void){
	conexion_con_suse = conectarse_a_suse();
	int opcode = INIT;
	int pid = getpid();
	void* buffer = malloc(2 * sizeof(int));
	memcpy(buffer, &opcode, sizeof(int));
	memcpy(buffer + sizeof(int), &pid, sizeof(int));
	send(conexion_con_suse, buffer, 2 * sizeof(int), MSG_WAITALL);
	init_internal(&hiloops);
}

hilo_t* crear_nuevo_hilo(int tid, int pid) {
	hilo_t* hilo = malloc(sizeof(hilo_t));
	hilo->tid = tid;
	hilo->pid = pid;
	hilo->tiempo_creacion = 0;
	hilo->tiempo_espera = 0;
	hilo->tiempo_cpu = 0;
	hilo->tiempo_ultima_llegada_a_ready = 0;
	hilo->tiempo_ultima_llegada_a_ready = 0;
	hilo->estimacion_anterior = 0;
	hilo->rafaga_anterior = 0;
	hilo->tid_hilo_a_esperar = 0;
	return hilo;
}
/*
int ejecutar_operacion(int tid, int operacion) {
	conexion_con_suse = conectarse_a_suse();
	int pid = getpid();
	t_paquete* paquete = crear_paquete(operacion);

	if (operacion == 3) {
		int tid_hilo_a_esperar = tid;
		hilo_t* hilo_a_esperar = crear_nuevo_hilo(tid_hilo_a_esperar, pid);
		tid = hilolay_get_tid();
		hilo_t* hilo = crear_nuevo_hilo(tid, pid);
		list_add(hilo->tid_hilo_esperando, hilo_a_esperar);
		agregar_a_paquete(paquete, hilo, sizeof(hilo_t));
		enviar_paquete(paquete, conexion_con_suse);
		return 0;
	}

	hilo_t* hilo = crear_nuevo_hilo(tid, pid);
	agregar_a_paquete(paquete, hilo, sizeof(hilo_t));
	enviar_paquete(paquete, conexion_con_suse);

	if (operacion == 2) {
		char* proximo = string_new();
		recv(conexion_con_suse, proximo, sizeof(proximo), MSG_WAITALL);
		return atoi(proximo);
	}

	return 0;
}
*/
int ejecutar_operacion_semaforo(int tid, char* sem_name, int operacion) {
	conexion_con_suse = conectarse_a_suse();
	int pid = getpid();
	t_paquete* paquete = crear_paquete(operacion);
	hilo_t* hilo = crear_nuevo_hilo(tid, pid);
	agregar_a_paquete(paquete, hilo, sizeof(hilo_t));
	enviar_paquete(paquete, conexion_con_suse);
	int tamanio = string_length(sem_name);
	send(conexion_con_suse, &tamanio, sizeof(int), MSG_WAITALL);
	send(conexion_con_suse, sem_name, tamanio, MSG_WAITALL);
	return 0;
}

int conectarse_a_suse() {
	struct sockaddr_in cliente;
	struct hostent *servidor;
	servidor = gethostbyname(IP);

	if(servidor == NULL)
	{
	  printf("Host erróneo\n");
	  return 1;
	}

	char buffer[100];
	int conexion = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *)&cliente, sizeof((char *)&cliente));

	cliente.sin_family = AF_INET;
	cliente.sin_port = htons(PUERTO);
	bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));

	if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	{
	  printf("Error conectando con el host\n");
	  close(conexion);
	  return 1;
	}

	return conexion;
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
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}
