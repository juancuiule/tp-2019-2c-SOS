#include "hilolay_alumnos.h"
#include "hilolay.h"

static struct hilolay_operations hiloops = {
		.suse_create = &suse_create,
		.suse_schedule_next = &suse_schedule_next,
		.suse_join = &suse_join,
		.suse_close = &suse_close
};

void hilolay_init() {
	init_internal(hilo_ops);
}

int* suse_create(int tid) {
	  struct sockaddr_in cliente;
	  struct hostent *servidor;
	  servidor = gethostbyname("localhost");
	  int puerto, conexion;
	  char buffer[100];
	  conexion = socket(AF_INET, SOCK_STREAM, 0);
	  bzero((char *)&cliente, sizeof((char *)&cliente));
	  cliente.sin_family = AF_INET;
	  cliente.sin_port = htons(8000);
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
	  connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)
	  printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  send(conexion, tid, sizeof(tid), 0);
}







