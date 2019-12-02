#include <stdio.h> 
#include <string.h> 
#include <errno.h> 
#include <stdlib.h> 
#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/lala.bin"

/*
 * PRUEBA ARCHIVO 10MB
 */	

int main ( ) {
    char buff[4096];
    FILE *in, *out;
    size_t  n;
 
    in = fopen( "file10m.bin", "rb" );
    out= fopen( PATHNAME, "a+" );
    while ( (n=fread(buff,1, 4096, in)) != 0 ) {
        fwrite( buff, 1, n, out );
    }
    return 0;
}