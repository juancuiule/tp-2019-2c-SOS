#include "client.h"

int main(void) {
	logger = log_create("../logs/client.log", "Cliente", 1, LOG_LEVEL_INFO);
	config = config_create("../program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	log_info(logger, "Conectandome a %s:%s", IP, PORT);

	int conexion = crear_conexion(IP, PORT);

	if (conexion == -1) {
		clean(conexion);
		return EXIT_FAILURE;
	}

	enviar_mensaje("Hola me conecte!", conexion);

	clean(conexion);
}