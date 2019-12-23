#include "confg.h"

t_config* c;

cli_config * config_leer(char* path){
	c = config_create(path);

	if(c == NULL){
		puts("Ruta de archivo no encontrada");
		exit(EXIT_FAILURE);
	}

	cli_config *config = malloc(sizeof(cli_config));

	config->cli_port = config_get_int_value(c, "CLI_PORT");
	config->sac_ip = config_get_string_value(c, "SAC_IP");
	config->sac_port = config_get_int_value(c, "SAC_PORT");

	return config;
}

void config_liberar(cli_config* config) {
	config_destroy(c);
	free(config);
	c=NULL;
}
