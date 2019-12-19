#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/hola.txt"
#define BUFFER "Arrived compass prepare an on as. Reasonable particular on my it in sympathize. Size now easy eat hand how. Unwilling he departure elsewhere dejection at. Heart large seems may purse means few blind. Exquisite newspaper attending on certainty oh suspicion of. He less do quit evil is. Add matter family active mutual put wishes happen. "


/*
 * PRUEBA OPEN WRITE Y CLOSE
 */	

int main(void)
{
	int leidos;
	int fd = open (PATHNAME, O_RDWR | O_APPEND);

	if (fd==-1)
	{
		perror("Error al abrir fichero:");
		exit(1);
	}

	printf("\nOperacion escritura de [ %d ] bytes", strlen(BUFFER));
	leidos = write(fd, BUFFER, strlen(BUFFER));
	printf ("\nSe escribio [ %d ] bytes.\n", leidos);

	close(fd);
	
	return 0;
}
