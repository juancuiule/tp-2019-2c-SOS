#include "server.h"

int atender_conexiones(int cliente_fd)
{
	package_t paquete;
	char *path;
	char *newpath;
	char *buffer;
	uint32_t blk;
	int flags;
	size_t size;
	off_t offset;

	while(true){
		paquete = paquete_recibir(cliente_fd);
		switch(paquete.header.cod_operacion){
			case COD_HANDSHAKE:
				handshake_enviar(cliente_fd, COD_PROCESO);
				break;
			case COD_OPENDIR:
				log_msje_info("Me llego operacion opendir");
				dslz_payload_with_path(paquete.payload, &path);
				sac_opendir(path, cliente_fd);
				break;
			case COD_READDIR:
				log_msje_info("Me llego operacion readdir");
				dslz_cod_readdir(paquete.payload, &path, &blk);
				sac_readdir(path, blk, cliente_fd);
				break;
			case COD_OPEN:
				log_msje_info("Me llego operacion open");
				dslz_cod_open(paquete.payload, &path, &flags);
				sac_open(path, flags, cliente_fd);
				break;
			case COD_GETATTR:
				log_msje_info("Me llego operacion getattr");
				dslz_payload_with_path(paquete.payload, &path);
				sac_getattr(path, cliente_fd);
				break;
			case COD_READ:
				log_msje_info("Me llego operacion read");
				dslz_cod_read(paquete.payload, &path, &blk, &size, &offset);
				sac_read(path, blk, size, offset, cliente_fd);
				break;
			case COD_MKDIR:
				log_msje_info("Me llego operacion mkdir");
				dslz_payload_with_path(paquete.payload, &path);
				sac_mkdir(path, cliente_fd);
				break;
			case COD_RMDIR:
				log_msje_info("Me llego operacion rmdir");
				dslz_payload_with_path(paquete.payload, &path);
				sac_rmdir(path,cliente_fd);
				break;
			case COD_MKNOD:
				log_msje_info("Me llego operacion mknod");
				dslz_payload_with_path(paquete.payload, &path);
				sac_mknod(path, cliente_fd);
				break;
			case COD_WRITE:
				log_msje_info("Me llego operacion write");
				dslz_cod_write(paquete.payload, &path, &buffer, &blk, &size, &offset);
				sac_write(path, buffer, blk, size, offset, cliente_fd);
				break;
			case COD_UNLINK:
				log_msje_info("Me llego operacion unlink");
				dslz_payload_with_path(paquete.payload, &path);
				sac_unlink(path,cliente_fd);
				break;
			case COD_TRUNCATE:
				log_msje_info("Me llego operacion truncate");
				dslz_cod_truncate(paquete.payload, &path, &offset);
				sac_truncate(path, offset, cliente_fd);
				break;
			case COD_RENAME:
				log_msje_info("Me llego operacion rename");
				dslz_cod_rename(paquete.payload, &path, &newpath);
				sac_rename(path, newpath, cliente_fd);
				break;
			case COD_DESC:
				log_msje_info("El cliente en el socket [ %d ] se desconecto", cliente_fd);
				return EXIT_FAILURE;
				break;
			default:
				log_msje_error("Codigo de operacion desconocido");
				break;
		}
	}
	return EXIT_SUCCESS;
}

void servidor_iniciar(int sac_port)
{
	int sac_fd = crear_servidor(sac_port);
	log_msje_info("Se creo el socket servidor [ %d ] en el puerto [ %d ]", sac_fd, sac_port);

	socket_t cliente;
	pthread_t thread_conexiones;

	while(true)
	{
		cliente.fd = aceptar_cliente(sac_fd);

		if(cliente.fd != -1)
		{
			log_msje_info("Se conecto socket [ %d ]", cliente.fd);

			pthread_create(&thread_conexiones,NULL, (void*)atender_conexiones, (void *)cliente.fd);
			pthread_join(thread_conexiones, NULL);
		}
	}

	socket_liberar(sac_fd);
}
