#include "main.h"

int main(int argc, char *argv[]) {
	log_iniciar("sac.log", "SAC SERVER", true);
	log_msje_info("Iniciando SAC Server");

	t_config* config = config_create(CONFG_PATH);
	int sac_port = config_get_int_value(config, "SAC_PORT");

	//Obtengo el disco.bin por parametros
	char *disk_name;
	if (argc == 2)
		disk_name = argv[1];
	else{
		log_msje_error("Debe ingresar el disco");
		return EXIT_FAILURE;
	}

	log_msje_info("disk name : [ %s ]", disk_name);

	//Mapeo el disco y seteo mis estructuras
	if(fs_map_disk_in_memory(disk_name)){
		fs_set_config();
	}
	else{
		log_msje_error("No se pudo mapear el disco a memoria");
		return EXIT_FAILURE;
	}

	servidor_iniciar(sac_port);

	fs_munmap_disk();
	log_liberar();
	return EXIT_SUCCESS;
}
