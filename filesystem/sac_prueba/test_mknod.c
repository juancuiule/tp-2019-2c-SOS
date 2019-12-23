#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#define FILENAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/hola.txt"

/*
 * PRUEBA MKNOD 
 */
int main(void)
{
	int result;
	result = mknod (FILENAME, S_IFREG, (dev_t)0);
	if (result < 0) {
	   perror ("mknod");
	   exit(1);
    }	
	return 0;
}
