#include "main.h"

int main(int argc, char *argv[]) {
	log_iniciar("sac.log", "SAC SERVER", true);
	log_msje_info("Iniciando SAC Server");

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
	if(map_disk_in_memory(disk_name)){
		filesystem_config();
	}
	else{
		log_msje_error("No se pudo mapear el disco a memoria");
		return EXIT_FAILURE;
	}

	servidor_iniciar();

	log_liberar();
	return EXIT_SUCCESS;
}
