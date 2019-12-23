#include "main.h"

cli_config* config;

int main(int argc, char *argv[])
{
	config = config_leer(CONFIG_PATH);
	log_iniciar("cli.log", "SAC CLI", true);
	log_msje_info("Iniciando SAC Cli");


	socket_t sac;//sac_server
	sac.fd = crear_conexion(config->sac_ip, config->sac_port);
	log_msje_info("Socket [ %d ] conectado a [ %s:%d  ]", sac.fd, config->sac_ip, config->sac_port);
	set_sac_fd(sac);

	handshake_enviar(sac.fd, COD_PROCESO);
	handshake_recibir(sac.fd);

	int res;
	res = fuse_main(argc, argv, &cli_oper, NULL);

	if(res!=0)
	{
		log_msje_error("Fuse main failure");
		return EXIT_FAILURE;
	}

	log_liberar();
	socket_liberar(sac.fd);
	config_liberar(config);
	return EXIT_SUCCESS;
}
