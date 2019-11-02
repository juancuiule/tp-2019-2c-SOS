#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <dirent.h>

#define TMP "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp"
#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/prueba.txt"
#define FILENAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/hola.txt"
#define FILEDIR "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/PRUEBADIR"
#define BUFFER "123456789"
#define SIZE 15
#define BSIZE 30

int main(void)
{
	/*
	 * PRUEBA OPEN READ WRITE Y CLOSE
	 */	
	/*
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
	
	close(fd);*/
	
	/*
	 * PRUEBA MKNOD
	 */
	/*
	int result;
	result = mknod (FILENAME, S_IFREG, (dev_t)0);
	if (result < 0) {
	   perror ("mknod");
	   exit(1);
	}*/

	/*
	 * PRUEBA OPENDIR READDIR MKDIR CLOSEDIR
	 */
	/*
	*/
    DIR *mydir;
    struct dirent *myfile;

    mydir = opendir(TMP);

	printf("\nListo el directorio \n");
    while((myfile = readdir(mydir)) != NULL)
    {
        printf("- %s\n", myfile->d_name);
    }

	printf("Creo nuevo directorio ..\n");
	int result;
	result = mkdir (FILEDIR, 0777);
	if (result < 0) {
	   perror ("mknod");
	   exit(1);
	}
	
	return 0;
}
