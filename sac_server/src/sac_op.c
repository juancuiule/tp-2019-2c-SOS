#include "sac_op.h"

//desc: completa el path con la ruta del sac
static void sac_fullpath(char fpath[PATH_MAX], char *path)
{
	 strcpy(fpath, FS_PATH);
	 strcat(fpath, path);
	 log_msje_info("full path : %s", fpath);
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
    	uint8_t state = sac_nodetable[blk].state;

    	log_msje_info("getattr: state = [ %d ]", state);
    	log_msje_info("getattr: size = [ %d ]", size);
    	log_msje_info("getattr: modif time = [ %d ]", modif_date);

    	paquete = slz_res_getattr(size, modif_date, state);
    }

    paquete_enviar(cliente_fd, paquete);
}

//desc: ejecuta op opendir en el fs local y envia respuesta a sac cli
void sac_opendir(char *path, int cliente_fd)
{
	package_t paquete;

	int error;

	int blk_number = fs_get_blk_by_fullpath(path);

    if (blk_number == -1)
    {
    	error = ENOENT;
    }
    else
    {
    	if(sac_nodetable[blk_number].state != 2)
    	{
    		error = ENOTDIR;
    	}
    	else
    	{
    		//dir_blk = (intptr_t)sac_nodetable + blk_number;
    		paquete = slz_res_opendir(blk_number);
    		paquete_enviar(cliente_fd, paquete);
    		return;
    	}
    }

	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);
	return;
}

//desc: ejecuta op readdir en el fs local y envia respuesta a sac cli
void sac_readdir(char *path, uint32_t blk_number, int cliente_fd)
{
	log_msje_info("SAC READDIR Path = [ %s ]", path);
	package_t paquete;

	t_list * filenames = list_create();
	fs_get_child_filenames_of(blk_number, filenames);

	paquete = slz_res_readdir(filenames);
	paquete_enviar(cliente_fd, paquete);

    log_msje_info("Exito operacion readdir sobre fs local");
}

void sac_mknod(char *path, int cliente_fd)
{
	log_msje_info("SAC MKNOD Path = [ %s ]", path);
	package_t paquete;
	int error;

	if(fs_path_exist(path))
	{
		error = EEXIST; //Pathname already exists
	}
	else
	{
		char *filename = get_last_filename_from_path(path);

		if(strlen(filename) > GFILENAMELENGTH)
		{
			error = ENAMETOOLONG; //Filename exceeds limit name length
		}
		else
		{
			char *prev_path = get_lastfile_previous_path(path);
			int father_blk = fs_get_blk_by_fullpath(prev_path);

			if (father_blk == -1)
			{
				error = ENOENT; //No such file or directory
			}
			else
			{
				if( sac_nodetable[father_blk].state != 2 )
				{
					error = ENOTDIR; //Component used as dir, is, in fact, not a dir
				}
				else
				{
					int node = fs_get_free_blk_node();

					if(node == EDQUOT)
					{
						error = EDQUOT; //Disk quota exceeded
					}
					else{
						log_msje_info("Encontre bloque libre, es el [ %d ]", node);
						GFile *node_to_set = sac_nodetable + node;
						node_to_set->state = 1;
						strcpy(node_to_set->fname, filename);
						node_to_set->parent_dir_block = father_blk;
						node_to_set->file_size = 0;
						node_to_set->c_date = get_current_time();
						node_to_set->m_date = get_current_time();
						//deberia asignarle un bloque de datos como minimo?

						paquete = slz_simple_res(COD_MKNOD);
						paquete_enviar(cliente_fd, paquete);
						return;
					}
				}
			}
		}
	}

	log_msje_error("mknod: [ %s ]", strerror(error));
	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);
	return;
}


void sac_releasedir(char *path, intptr_t dir, int cliente_fd)
{
	log_msje_info("SAC CLOSEDIR Path = [ %s ]", path);
	package_t paquete = paquete = slz_simple_res(COD_RELEASEDIR);
	//paquete = slz_res_error(ENOENT);
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

void sac_mkdir(char *path, int cliente_fd)
{
	log_msje_info("SAC MKDIR Path = [ %s ]", path);
	/*
	package_t paquete;

	char fpath[PATH_MAX];
	sac_fullpath(fpath, path);

	int res, err;
	//ejecuto operacion
	//res = mkdir(fpath, mode);

    if (res == -1) {
    	log_msje_error("mkdir: [ %s ]", strerror(errno));
    	err = errno;
    	paquete = slz_res_error(err);
    }
    else {
    	log_msje_info("Exito operacion mkdir sobre fs local");
    	paquete = slz_simple_res(COD_MKDIR);
    }

    paquete_enviar(cliente_fd, paquete);*/
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

void sac_unlink(char *path, int cliente_fd)
{
	log_msje_info("SAC UNLINCK = [ %s ]", path);
	package_t paquete;
	int res_unlink, err;

	char full_path[PATH_MAX];
	sac_fullpath(full_path, path);

	res_unlink = unlink(path);

	if(res_unlink == -1){
		log_msje_error("unlink: [ %s ]", strerror(errno));
		err = errno;
		paquete = slz_res_error(err);
	}
	else {
		log_msje_info("Exito operacion unlink sobre fs local");
		paquete = slz_simple_res(COD_UNLINK);
	}

	paquete_enviar(cliente_fd, paquete);

}

