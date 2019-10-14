#include "alumnos.h"

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>

/* Lib implementation: It'll only schedule the last thread that was created */
int max_tid = 0;

int enviar_mensaje() {
	struct sockaddr_in cliente; //Declaración de la estructura con información para la conexión
	  struct hostent *servidor; //Declaración de la estructura con información del host
	  servidor = gethostbyname("localhost"); //Asignacion
	  if(servidor == NULL)
	  { //Comprobación
	    printf("Host erróneo\n");
	    return 1;
	  }
	  int puerto, conexion;
	  char buffer[100];
	  conexion = socket(AF_INET, SOCK_STREAM, 0); //Asignación del socket
	  puerto=8000; //conversion del argumento
	  bzero((char *)&cliente, sizeof((char *)&cliente)); //Rellena toda la estructura de 0's
	        //La función bzero() es como memset() pero inicializando a 0 todas la variables
	  cliente.sin_family = AF_INET; //asignacion del protocolo
	  cliente.sin_port = htons(puerto); //asignacion del puerto
	  bcopy((char *)servidor->h_addr, (char *)&cliente.sin_addr.s_addr, sizeof(servidor->h_length));
	  //bcopy(); copia los datos del primer elemendo en el segundo con el tamaño máximo
	  //del tercer argumento.


	  //cliente.sin_addr = *((struct in_addr *)servidor->h_addr); //<--para empezar prefiero que se usen
	  //inet_aton(argv[1],&cliente.sin_addr); //<--alguna de estas dos funciones
	  if(connect(conexion,(struct sockaddr *)&cliente, sizeof(cliente)) < 0)
	  { //conectando con el host
	    printf("Error conectando con el host\n");
	    close(conexion);
	    return 1;
	  }
	  printf("Conectado con %s:%d\n",inet_ntoa(cliente.sin_addr),htons(cliente.sin_port));
	  //inet_ntoa(); está definida en <arpa/inet.h>
	  printf("Escribe un mensaje: ");
	  fgets(buffer, 100, stdin);
	  send(conexion, buffer, 100, 0); //envio
	  bzero(buffer, 100);
	  recv(conexion, buffer, 100, 0); //recepción
	  printf("%s", buffer);
	return 0;
}

int suse_create(int tid){
	if (tid > max_tid) max_tid = tid;
	enviar_mensaje("Hola, me estoy conectando desde hilolay.\n");
	return 0;
}

int suse_schedule_next(void){
	int next = max_tid;
	printf("Scheduling next item %i...\n", next);
	return next;
}

int suse_join(int tid){
	// Not supported
	return 0;
}

int suse_close(int tid){
	printf("Closed thread %i\n", tid);
	max_tid--;
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
