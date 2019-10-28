#include "sac_op.h"

//desc: completa el path con la ruta del sac
static void sac_fullpath(char fpath[PATH_MAX], char *path)
{
	 strcpy(fpath, FS_PATH);
	 strcat(fpath, path);
	 log_msje_info("full path : %s", fpath);
}

//desc: ejecuta op opendir en el fs local y envia respuesta a sac cli
void sac_opendir(char *path, int cliente_fd)
{
	DIR *dp;
	char fpath[PATH_MAX];
	package_t paquete;

	//completo path
	sac_fullpath(fpath, path);

	//ejecuto operacion
    dp = opendir(fpath);

    if (dp == NULL)
    {
    	log_msje_error("sac_opendir opendir");
    	log_msje_error("opendir: [ %s ]", strerror(errno));
    	paquete = slz_res_opendir(NULL, true);//Agregar tipo de error
    }
    else
    	paquete = slz_res_opendir(dp, false);

    paquete_enviar(cliente_fd, paquete);//funciona falta validar
}

//desc: ejecuta op readdir en el fs local y envia respuesta a sac cli
void sac_readdir(char *path, intptr_t dir, int cliente_fd)
{
	log_msje_info("SAC READDIR Path = [ %s ]", path);
	package_t paquete;
	DIR *dp;
	dp = (DIR *) dir;

	struct dirent *de;

	//ejecuto operacion
	de = readdir(dp);//o mejor usar readdir_r

    if (de == NULL) {//error
    	log_msje_error("sac readdir");
    	log_msje_error("readdir: [ %s ]", strerror(errno));
    	paquete = slz_res_readdir(NULL, true);
    }
    else {
    	log_msje_info("Exito operacion readdir sobre fs local");

    	t_list * filenames = list_create();
    	do {
			list_add(filenames, de->d_name);
		} while ((de = readdir(dp)) != NULL);

    	paquete = slz_res_readdir(filenames, false);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_releasedir(char *path, intptr_t dir, int cliente_fd)
{
	log_msje_info("SAC CLOSEDIR Path = [ %s ]", path);
	package_t paquete;
	DIR *dp;
	dp = (DIR *) dir;

	int res;
	//ejecuto operacion
	res = closedir(dp);

    if (res == -1) {
    	log_msje_error("closedir: [ %s ]", strerror(errno));
    	paquete = slz_res_releasedir(true);
    }
    else {//todo ok
    	log_msje_info("Exito operacion closedir sobre fs local");
    	paquete = slz_res_releasedir(false);
    }

    paquete_enviar(cliente_fd, paquete);

}

void sac_open(char *path, int flags, int cliente_fd)
{
	log_msje_info("SAC OPEN Path = [ %s ]", path);
	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int fd;
	//ejecuto operacion
	fd = open(fpath, flags);

    if (fd == -1) {
    	log_msje_error("open: [ %s ]", strerror(errno));
    	paquete = slz_res_open(fd, true);
    }
    else {//todo ok
    	log_msje_info("Exito operacion open sobre fs local");
    	paquete = slz_res_open(fd, false);
    }

    paquete_enviar(cliente_fd, paquete);

}

void sac_getattr(char *path, int cliente_fd)
{
	log_msje_info("SAC GETATTR Path = [ %s ]", path);
	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int res, err;
	struct stat stbuf;
	//ejecuto operacion
	res = lstat(fpath, &stbuf);

    if (res == -1) {
    	log_msje_error("getattr: [ %s ]", strerror(errno));
    	err = errno;
    	if (err == ENOENT){
    		log_msje_error("ENOENT");
    	}
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion getattr sobre fs local");
    	paquete = slz_res_getattr(stbuf.st_mode, stbuf.st_nlink, stbuf.st_size);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_read(char *path, int fd, size_t size, off_t offset, int cliente_fd)
{
	log_msje_info("SAC READ Path = [ %s ]", path);
	package_t paquete;

	int leido;
	char * buffer = malloc(size);

	//ejecuto operacion
	leido = pread(fd, buffer, size, offset);

    if (leido == -1) {
    	log_msje_error("pread: [ %s ]", strerror(errno));
    	paquete = slz_res_read(buffer, leido, true);
    }
    else {
    	log_msje_info("Exito operacion pread sobre fs local");
    	paquete = slz_res_read(buffer, leido, false);
    }

    paquete_enviar(cliente_fd, paquete);
    free(buffer);
}

void sac_release(char *path, int fd, int cliente_fd)
{
	log_msje_info("SAC RELEASE Path = [ %s ]", path);
	package_t paquete;

	int res;
	//ejecuto operacion
	res = close(fd);

    if (res == -1) {
    	log_msje_error("close: [ %s ]", strerror(errno));
    	paquete = slz_res_release(true);
    }
    else {
    	log_msje_info("Exito operacion close sobre fs local");
    	paquete = slz_res_release(false);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_mkdir(char *path, uint32_t mode, int cliente_fd)
{
	log_msje_info("SAC MKDIR Path = [ %s ]", path);

	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int res;
	//ejecuto operacion
	res = mkdir(fpath, mode);

    if (res == -1) {
    	log_msje_error("mkdir: [ %s ]", strerror(errno));
    	paquete = slz_res_mkdir(true);
    }
    else {
    	log_msje_info("Exito operacion mkdir sobre fs local");
    	paquete = slz_res_mkdir(false);
    }

    paquete_enviar(cliente_fd, paquete);
}
