#include "main.h"

void* atender_conexiones(void* data){
	pthread_data* data_thread = (pthread_data*) data;
	int cliente_fd = data_thread->socket;
	free(data_thread);

	package_t paquete;
	char *path;

	while(true){
		paquete = paquete_recibir(cliente_fd);
		switch(paquete.header.cod_operacion){
			case COD_HANDSHAKE:
				handshake_enviar(cliente_fd, COD_PROCESO);
				break;
			case COD_READDIR:
				log_msje_info("Me llego operacion readdir");
				dslz_cod_readdir(paquete.payload, &path);
				log_msje_info("Path: %s", path);
				break;
			default:
				log_msje_error("Codigo de operacion erroneo");
				break;

		}

	}
}

int main(void) {
	log_iniciar("sac.log", "SAC SERVER", true);
	log_msje_info("Iniciando SAC Server");

	socket_t sac;
	sac.fd = crear_servidor(SAC_PORT);
	log_msje_info("Se creo el socket servidor [ %d ] en el puerto [ %d ]", sac.fd, SAC_PORT);

	socket_t cliente;
	pthread_t thread_conexiones;

	while(true)
	{
		cliente.fd = aceptar_cliente(sac.fd);

		if(cliente.fd != -1)
		{
			log_msje_info("Se conecto socket [ %d ]", cliente.fd);

			pthread_data* data_thread = (pthread_data*) malloc(sizeof(*data_thread));
			data_thread->socket = cliente.fd;

			pthread_create(&thread_conexiones,NULL, (void*)atender_conexiones, data_thread);
			pthread_join(thread_conexiones, NULL);
		}
	}

	socket_liberar(sac.fd);
	log_liberar();
	return EXIT_SUCCESS;
}
