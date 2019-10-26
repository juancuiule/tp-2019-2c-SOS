#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/prueba.txt"
#define SIZE 23

/*
 * PRUEBA OPEN READ Y CLOSE
 */

int main(void)
{
   char cadena[SIZE];
   int leidos;

   int fichero = open (PATHNAME, O_RDONLY);

   if (fichero==-1)
   {
        perror("Error al abrir fichero:");
        exit(1);
   }

   leidos = read(fichero, cadena, SIZE);

   close(fichero);

   cadena[SIZE]='\0';

   printf ( "Se leyeron [ %d ] bytes. La cadena leída es [ %s ]\n", leidos, cadena );

   return 0;
}
