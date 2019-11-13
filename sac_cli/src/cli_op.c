#include "cli_op.h"

//Primer operacion consultando al disco binario!
int cli_getattr(const char *path, struct stat *statbuf)
{
	log_msje_info("Operacion GETATTR sobre path [ %s ]", path);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_path_with_cod(path, COD_GETATTR);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion getattr al server");

	//...espero respuesta de server
	uint32_t size;
	uint64_t m_date;
	int state, errnum;

	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		log_msje_error("getattr me llego cod error");
		dslz_res_error(respuesta.payload, &errnum);
		return -errnum;
	}

	dslz_res_getattr(respuesta.payload, &size, &m_date, &state);

	struct timespec ts_m_time;
	convert_to_timespec(m_date, &ts_m_time);

	statbuf->st_size = size;
	statbuf->st_mtim = ts_m_time;

	if(state == 1)
		statbuf->st_mode = S_IFREG | 0777;
	else//state solo puede ser 2
		statbuf->st_mode = S_IFDIR | 0755;

	return 0;
}

//OK
int cli_opendir(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion OPENDIR sobre path [ %s ]", path);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_path_with_cod(path, COD_OPENDIR);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion readdir al server");

	//...espero respuesta de server
	uint32_t dir;
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("opendir me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	dslz_res_opendir(respuesta.payload, &dir);
	log_msje_info("CLI OPENDIR  ME LLEGO BLK NUMBER: [ %d ]", dir);
	fi->fh = dir;//la guardoo

	return 0;

}

//OK
int cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion READDIR sobre path [ %s ]", path);
	package_t paquete, respuesta;

	/*
	if((uint32_t)fi->fh == NULL)
		return -EBADF;*/

	log_msje_info("-- readdir bloquee nro [ %d ]", fi->fh);
	paquete = slz_cod_readdir(path, fi->fh);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion readdir al server");

	//..espero respuestaa server
	respuesta = paquete_recibir(sac_server.fd);

	t_list *files = list_create();
	dslz_res_readdir(respuesta.payload, &files);

	if(!list_is_empty(files))
	{
		t_link_element *element = files->head;
		t_link_element *aux = NULL;
		do{
			aux = element->next;

			log_msje_info("calling filler with element data list: [ %s ]", element->data);
			if (filler(buf, element->data, NULL, 0) != 0) {
				log_msje_error("cli readdir filler:  buffer full");
				return -ENOMEM;
			}

			element = aux;

		}while(element != NULL);
	}
	return 0;
}

//OK
int cli_releasedir(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion RELEASEDIR sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_cod_releasedir(path, (uint32_t)fi->fh);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod realesedir");
	else
		log_msje_info("Se envio operacion releasedir al server");

	//...espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("releasedir me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	return 0;
}

//OK
int cli_open(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion OPEN sobre path [ %s ]", path);

	//le paso el path y fi-flags
	package_t paquete, respuesta;
	paquete = slz_cod_open(path, fi->flags);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod open");
	else
		log_msje_info("Se envio operacion open al server");

	//espero respuesta de server : un filedescriptor
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("opend me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	int fd;
	dslz_res_open(respuesta.payload, &fd);

	//Me guardo en fi el fd
	fi->fh = fd;

	return 0;
}


//OK
int cli_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion READ sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_cod_read(path, fi->fh, size, offset);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod read");
	else
		log_msje_info("Se envio operacion read al server");

	//espero respuesta de server : un size y el buffer leido
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("read me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	int leido;
	dslz_res_read(respuesta.payload, buf, &leido);

	return leido;
}

int cli_release(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion RELEASE sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_cod_release(path, fi->fh);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod release");
	else
		log_msje_info("Se envio operacion release al server");

	//espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("release me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	return 0;
}

int cli_flush(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion FLUSH sobre path [ %s ]", path);
	return 0;
}

int cli_mkdir(const char *path, mode_t mode)
{
	log_msje_info("Operacion MKDIR sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_path_with_cod(path, COD_MKDIR);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod mkdir");
	else
		log_msje_info("Se envio operacion mkdir al server");

	//espero respuesta del server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("mkdir me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	return 0;
}

int cli_rmdir(const char *path)
{
	log_msje_info("Operacion RMDIR sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_path_with_cod(path, COD_RMDIR);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod rmdir");
	else
		log_msje_info("Se envio operacion rmdir al server");

	//espero respuesta del server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("rmdir me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	return 0;
}

int cli_mknod(const char *filename, mode_t mode, dev_t dev)
{
	log_msje_info("Operacion MKNOD creando el archivo [ %s ]", filename);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_path_with_cod(filename, COD_MKNOD);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion mknod al server");

	//...espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("mknod me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	log_msje_info("CLI MKNOD  CREE UN ARCHIVO: [ %s ]", filename);
	return 0;
}

int cli_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion WRITE sobre el archivo ");

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_cod_write(path, buf, fi->fh, size, offset);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion write al server");

	//...espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("write me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	int leido;
	dslz_res_write(respuesta.payload, &leido);

	return leido;
}

int cli_unlink(const char *path)
{
	log_msje_info("Operacion UNLINK sobre path [ %s ]", path);

	package_t paquete, respuesta;
	paquete = slz_path_with_cod(path, COD_UNLINK);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete cod unlink");
	else
		log_msje_info("Se envio operacion unlink al server");

	//espero respuesta del server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		int err;
		log_msje_error("unlink me llego cod error");
		dslz_res_error(respuesta.payload, &err);
		return -err;
	}

	return 0;
}

void set_sac_fd(socket_t socket)
{
	sac_server = socket;
}

struct fuse_operations cli_oper = {
		.getattr = cli_getattr,
		.opendir = cli_opendir,
		.readdir = cli_readdir,
		//.releasedir = cli_releasedir,
		.open = cli_open,
		.read = cli_read,
		.release = cli_release,
		.flush = cli_flush,
		.mkdir = cli_mkdir,
		.rmdir = cli_rmdir,
		.mknod = cli_mknod,
		.write = cli_write,
		.unlink = cli_unlink
};
