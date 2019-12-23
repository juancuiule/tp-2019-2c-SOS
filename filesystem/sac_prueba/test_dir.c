#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#define TMP "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp"
#define FILEDIR "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/PRUEBADIR"

/*
 * PRUEBA OPENDIR READDIR MKDIR CLOSEDIR
 */

int main(void)
{
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
