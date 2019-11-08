#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/prueba.txt"
#define BUFFER "123456789"
#define SIZE 15
#define BSIZE 30

/*
 * PRUEBA OPEN READ WRITE Y CLOSE
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
	
	buffer[strlen(buffer)]='\0';
	printf ("\nSe leyeron [ %d ] bytes. La cadena leída es [ %s ]", leidos, buffer );

	printf("\nOperacion escritura de [ %d ] bytes", strlen(BUFFER));
	leidos = write(fd, BUFFER, strlen(BUFFER));
	printf ("\nSe escribio [ %d ] bytes.\n", leidos);
	
	close(fd);
	
	return 0;
}
