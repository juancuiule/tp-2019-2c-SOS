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
    	log_msje_error("opendir: [ %s ]", strerror(errno));
    	int err = errno;
    	paquete = slz_res_error(err);
    }
    else
    	paquete = slz_res_opendir(dp);

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

    if (de == NULL) {
    	log_msje_error("readdir: [ %s ]", strerror(errno));
    	int err=errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion readdir sobre fs local");

    	t_list * filenames = list_create();
    	do {
			list_add(filenames, de->d_name);
		} while ((de = readdir(dp)) != NULL);

    	paquete = slz_res_readdir(filenames);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_releasedir(char *path, intptr_t dir, int cliente_fd)
{
	log_msje_info("SAC CLOSEDIR Path = [ %s ]", path);
	package_t paquete;
	DIR *dp;
	dp = (DIR *) dir;

	int res, err;
	//ejecuto operacion
	res = closedir(dp);

    if (res == -1) {
    	log_msje_error("closedir: [ %s ]", strerror(errno));
		err = errno;
		paquete = slz_res_error(err);
    }
    else {//todo ok
    	log_msje_info("Exito operacion closedir sobre fs local");
    	paquete = slz_simple_res(COD_RELEASEDIR);
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
    	int err=errno;
    	paquete = slz_res_error(err);
    }
    else {//todo ok
    	log_msje_info("Exito operacion open sobre fs local");
    	paquete = slz_res_open(fd);
    }

    paquete_enviar(cliente_fd, paquete);

}


/*
 * Primer operacion que va a consultar con nuestro disco binario sac!
 *
 * Mandamos size y modif time
 */
void sac_getattr(char *path, int cliente_fd)
{
	log_msje_info("SAC GETATTR Path = [ %s ]", path);
	package_t paquete;

	int blk;

	//Buscamos el bloque que le corresponde al archivo
	blk = fs_get_blk_by_fullpath(path);

    if (blk == -1) {
    	log_msje_error("getattr: no such file or directory");
    	paquete = slz_res_error(ENOENT);
    }
    else {
    	log_msje_info("Exito operacion getattr sobre disco binario");
    	uint32_t size= sac_nodetable[blk].file_size;
    	uint64_t modif_date = sac_nodetable[blk].m_date;

    	paquete = slz_res_getattr(size, modif_date);
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
    	int err=errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion pread sobre fs local");
    	paquete = slz_res_read(buffer, leido);
    }

    paquete_enviar(cliente_fd, paquete);
    free(buffer);
}

void sac_release(char *path, int fd, int cliente_fd)
{
	log_msje_info("SAC RELEASE Path = [ %s ]", path);
	package_t paquete;

	int res, err;
	//ejecuto operacion
	res = close(fd);

    if (res == -1) {
    	log_msje_error("close: [ %s ]", strerror(errno));
    	err = errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion close sobre fs local");
    	paquete = slz_simple_res(COD_RELEASE);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_mkdir(char *path, uint32_t mode, int cliente_fd)
{
	log_msje_info("SAC MKDIR Path = [ %s ]", path);

	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int res, err;
	//ejecuto operacion
	res = mkdir(fpath, mode);

    if (res == -1) {
    	log_msje_error("mkdir: [ %s ]", strerror(errno));
    	err = errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion mkdir sobre fs local");
    	paquete = slz_simple_res(COD_MKDIR);
    }

    paquete_enviar(cliente_fd, paquete);
}

void sac_rmdir(char *path, int cliente_fd)
{
	log_msje_info("SAC RMDIR Path = [ %s ]", path);

	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int res, err;
	//ejecuto operacion
	res = rmdir(fpath);

    if (res == -1) {
    	log_msje_error("mkdir: [ %s ]", strerror(errno));
    	err = errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion rmdir sobre fs local");
    	paquete = slz_simple_res(COD_RMDIR);
    }

    paquete_enviar(cliente_fd, paquete);

}

void sac_mknod(char *path, mode_t mode, dev_t dev, int cliente_fd){

	log_msje_info("SAC MKNOD Path = [ %s ]", path);
	package_t paquete;
	int res_mknod, err;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);
	//primero me ubico en el directorio
	// ver si no uso directo opendir?
	//sac_opendir(path, cliente_fd);

	//ejecuta la operacion crear un archivo
	res_mknod = mknod(fpath, mode, dev);

	//valido la respuesta de la operacion
	if(res_mknod == -1){
		log_msje_error("mknod: [ %s ]", strerror(errno));
		err = errno;
		paquete = slz_res_error(err);
	}
	else
		paquete = slz_simple_res(COD_MKNOD);

	paquete_enviar(cliente_fd, paquete);


}

void sac_write(char *path, char *buffer, int fd, size_t size, off_t offset, int cliente_fd)
{
	log_msje_info("SAC WRITE Path = [ %s ]", path);
	package_t paquete;

	int leido;

	//ejecuto operacion
	leido = write(fd, buffer, size);

    if (leido == -1) {
    	log_msje_error("pwrite: [ %s ]", strerror(errno));
    	int err=errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion pwrite sobre fs local");
    	paquete = slz_res_write(leido);
    }

    paquete_enviar(cliente_fd, paquete);
}

