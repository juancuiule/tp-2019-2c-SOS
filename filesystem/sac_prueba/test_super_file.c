#include <stdio.h> 
#include <string.h> 
#include <errno.h> 
#include <stdlib.h> 
#define PATHNAME "/home/utnso/workspace/tp-2019-2c-SOS/sac_cli/Debug/tmp/elsuperfile.txt"

/*
 * PRUEBA SUPER ARCHIVO
 */	

int main() 
{ 
    FILE *fd_A, *fd_B; 
  
    // Open one file for reading 
    fd_A = fopen("superfile.txt", "r"); 
    if (fd_A == NULL) 
    { 
        printf("Cannot open file ...\n"); 
        exit(0); 
    } 
  
    // Open another file for writing 
    fd_B = fopen(PATHNAME, "a+"); 
    if (fd_B == NULL) 
    { 
        printf("%s\n", strerror(errno));
        printf("Cannot open file tmp ... \n"); 
        exit(0); 
    } 
  
    // Read contents from file 
    char c = fgetc(fd_A); 
    while (c != EOF) 
    { 
        fputc(c, fd_B); 
        c = fgetc(fd_A); 
    } 
  
    printf("\nContents copied\n"); 
  
    fclose(fd_A); 
    fclose(fd_B); 
    return 0; 
}
