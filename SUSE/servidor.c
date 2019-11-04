/*
 * server.c
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#include "dispatcher.h"
#include "configuracion.h"
#include "servidor.h"
#include "utils.h"

void inicializar_colas() {
	cola_new = queue_create();
	cola_blocked = queue_create();
	cola_exit = queue_create();
}

int servidor() {
	int server_fd = init_server(LISTEN_IP, LISTEN_PORT);
	pthread_t hilo;

	while(1) {
		int client_fd = recibir_cliente(server_fd);
		pthread_create(&hilo, NULL, (void*)respond_to_client, (void*)client_fd);
	}
}

ult_t* deserializar(void* buffer) {
	ult_t *ult = malloc(1000);
	memcpy(&(ult->pid), buffer, sizeof(ult->pid));
	memcpy(&(ult->tid), buffer + 1, sizeof(ult->tid));
	return ult;
}

void encolar_ult(int tid) {
	queue_push(cola_new, tid);
	printf("SUSE: nuevo ULT en NEW (TID: %i)\n", tid);
}

void mostrar_ults_cola(t_queue* cola) {
	int tid = 0;
	t_queue* aux = malloc(sizeof(t_queue));
	aux = cola;
	printf("SUSE: ULTs en cola: ");

	while (aux->elements->head->next != NULL) {
		tid = queue_peek(aux);
		printf("%i - ", tid);
		aux = aux->elements->head->next;
	}

	free(aux);
}










