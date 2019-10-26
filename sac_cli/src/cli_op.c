#include "cli_op.h"

//ejemplo getattr, falta,tiene que conectarse al server
int cli_getattr(const char *path, struct stat *stbuf)
{
	int res = 0;

	memset(stbuf, 0, sizeof(struct stat));

	if (strcmp(path, "/") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, "/memes") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else if (strcmp(path, "/notas") == 0) {
		stbuf->st_mode = S_IFDIR | 0755;
		stbuf->st_nlink = 2;
	} else {
		res = -ENOENT;
	}

	return res;
}

int cli_opendir(const char *path, struct fuse_file_info *fi)
{
	log_msje_info("Operacion OPENDIR sobre path [ %s ]", path);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_cod_opendir(path);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion readdir al server");

	//...espero respuesta de server
	intptr_t dir;
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		log_msje_error("opendir me llego cod error");
		return -1;
	}

	dslz_res_opendir(respuesta.payload, &dir);
	log_msje_info("CLI OPENDIR  ME LLEGO DIR ADRESS: [ %p ]", dir);
	fi->fh = dir;//la guardoo

	return 0;

}
//esta op ya se esta conectando con el sac server
int cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion READDIR sobre path [ %s ]", path);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_cod_readdir(path, (intptr_t)fi->fh);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion readdir al server");

	//..espero respuestaa server
	t_list *files = list_create();
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		log_msje_error("readdir me llego cod error");
		return -1;
	}

	dslz_res_readdir(respuesta.payload, &files);

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


	return 0;
}


int cli_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion read sobre path %s", path);

	return 0;
}

int cli_mkdir(const char *path, mode_t mode)
{
	log_msje_info("mkdir");

    return 0;
}

int cli_mknod(const char *filename, mode_t mode, dev_t dev){
	log_msje_info("Operacion MKNOD creando el archivo [ %s ]", filename);

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_cod_mknod(filename, mode, dev);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion mknod al server");

	//...espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		log_msje_error("mknod me llego cod error");
		return -1;
	}
	dslz_res_mknode(respuesta.payload);
	log_msje_info("CLI MKNOD  CREE UN ARCHIVO: [ %s ]", filename);


	return 0;
}

int cli_write(int filedes, const void* buffer, size_t size_t){
	log_msje_info("Operacion WRITE creando el archivo ");

	//enviar paquete a sac server
	package_t paquete, respuesta;
	paquete = slz_cod_write(filedes, &buffer, size_t);

	if(!paquete_enviar(sac_server.fd, paquete))
			log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion write al server");

	//...espero respuesta de server
	respuesta = paquete_recibir(sac_server.fd);

	if(respuesta.header.cod_operacion == COD_ERROR){
		log_msje_error("write me llego cod error");
		return -1;
	}
	dslz_res_write(respuesta.payload);
	log_msje_info("CLI WRITE  ESCRIBI UN ARCHIVO");

	//ver si no necesita el path del archivo? o el server lo conoce?
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
		.read = cli_read,
		.mkdir = cli_mkdir,
		.mknod = cli_mknod,
		.write = cli_write
};
