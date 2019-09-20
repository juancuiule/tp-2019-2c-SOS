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

//esta op ya se esta conectando con el sac server
int cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi)
{
	log_msje_info("Operacion READDIR sobre path %s", path);

	//enviar paquete a sac server
	package_t paquete;
	paquete = slz_cod_readdir(path);

	if(!paquete_enviar(sac_server.fd, paquete))
		log_msje_error("No se pudo enviar el paquete");
	else
		log_msje_info("Se envio operacion readdir al server");

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

void set_sac_fd(socket_t socket)
{
	sac_server = socket;
}

struct fuse_operations cli_oper = {
		.getattr = cli_getattr,
		.readdir = cli_readdir,
		.read = cli_read,
		.mkdir = cli_mkdir,
};
