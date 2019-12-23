#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/hola.txt"
#define BSIZE 45

/*
 * PRUEBA OPEN READ Y CLOSE
 */	

int main(void)
{
	char buffer[BSIZE];
	int leidos;

	int fd = open (PATHNAME, O_RDWR | O_APPEND);

	if (fd==-1)
	{
		perror("Error al abrir fichero:");
		exit(1);
	}

	printf("\nOperacion lectura de [ %d ] bytes", BSIZE);
	leidos = read(fd, buffer, BSIZE);
	
	buffer[leidos]='\0';
	printf ("\nSe leyeron [ %d ] bytes. La cadena leída es [ %s ]\n", leidos, buffer );


	close(fd);
	
	return 0;
}
