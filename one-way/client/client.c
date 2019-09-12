#include "client.h"

void clean(int conexion);

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

	log_info(logger, "Mi socket es: %i", conexion);

	enviar_mensaje("Hola me conecte!", conexion);

	clean(conexion);
}

void clean(int conexion) {
	log_destroy(logger);
	config_destroy(config);
	liberar_conexion(conexion);
}