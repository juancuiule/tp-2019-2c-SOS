#ifndef CONFG_H_
#define CONFG_H_

#include <commons/config.h>
#include <stdio.h>
#include <stdlib.h>


typedef struct{
	int cli_port;
	char *sac_ip;
	int sac_port;

}cli_config;

cli_config *config_leer(char* path);
void config_liberar(cli_config* config);

#endif /* CONFG_H_ */
