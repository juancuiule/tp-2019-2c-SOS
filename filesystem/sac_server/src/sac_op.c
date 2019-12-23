#include "sac_op.h"

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
    	uint32_t size = sac_nodetable[blk].file_size;
    	uint64_t modif_date = sac_nodetable[blk].m_date;
    	uint8_t state = sac_nodetable[blk].state;

    	log_msje_info("getattr: state = [ %d ]", state);
    	log_msje_info("getattr: size = [ %d ]", size);
    	log_msje_info("getattr: modif time = [ %lu ]", modif_date);

    	paquete = slz_res_getattr(size, modif_date, state);
    }

    paquete_enviar(cliente_fd, paquete);
}

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
						error = EDQUOT; //Disk inodes exceeded
					}
					else
					{
						log_msje_info("Encontre nodo libre, es el [ %d ]", node);

						ptrGBloque blk_ind = fs_get_blk_ind_with_data_blk();

						if(blk_ind == EDQUOT)
						{
							error = EDQUOT; //Disk blocks exceeded
						}
						else
						{
							GFile *node_to_set = sac_nodetable + node;
							node_to_set->state = 1;
							strcpy(node_to_set->fname, filename);
							node_to_set->parent_dir_block = father_blk;
							node_to_set->file_size = 0;
							node_to_set->c_date = get_current_time();
							node_to_set->m_date = get_current_time();
							node_to_set->blk_indirect[0] = blk_ind;

							paquete = slz_simple_res(COD_MKNOD);
							paquete_enviar(cliente_fd, paquete);
							return;

						}
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


void sac_open(char *path, int flags, int cliente_fd)
{
	log_msje_info("SAC OPEN Path = [ %s ]", path);
	package_t paquete;
	int error;

	int blk_number = fs_get_blk_by_fullpath(path);

    if (blk_number == -1)
    {
    	error = ENOENT;
    }
    else
    {
    	if(sac_nodetable[blk_number].state == 2)
    	{
    		error = EISDIR; //Pathname refers to a directory
    	}

    	if(sac_nodetable[blk_number].state == 1)
    	{
    		log_msje_info("Exito operacion open sobre disco");
    		paquete = slz_res_open(blk_number);
    		paquete_enviar(cliente_fd, paquete);
    		return;
    	}
    }

	log_msje_error("open: [ %s ]", strerror(error));
	paquete = slz_res_error(error);

    paquete_enviar(cliente_fd, paquete);
    return;

}

void sac_read(char *path, uint32_t blk, size_t size, off_t offset, int cliente_fd)
{
	log_msje_info("SAC READ Path = [ %s ]", path);
	package_t paquete;

	int error;
	if(offset > sac_nodetable[blk].file_size)
	{
		error = EFAULT;
		log_msje_error("read: [ %s ]", strerror(error));
		paquete = slz_res_error(error);
		paquete_enviar(cliente_fd, paquete);
		return;
	}

	//Si quiere leer mas de lo que puede, restringo el size, a lo que puede leer
	if(offset + size > sac_nodetable[blk].file_size) size = sac_nodetable[blk].file_size - offset;

	int leido;
	char * buffer = malloc(size);

	leido = fs_read_file(buffer, size, offset, blk);

	log_msje_info("Exito operacion read sobre disco");
	paquete = slz_res_read(buffer, leido);
    paquete_enviar(cliente_fd, paquete);
    free(buffer);
}

void sac_mkdir(char *path, int cliente_fd)
{
	log_msje_info("SAC MKDIR Path = [ %s ]", path);
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
						error = EDQUOT; //Disk inodes exceeded
					}
					else
					{
						log_msje_info("Encontre nodo libre, es el [ %d ]", node);

						GFile *node_to_set = sac_nodetable + node;
						node_to_set->state = 2;
						strcpy(node_to_set->fname, filename);
						node_to_set->parent_dir_block = father_blk;
						node_to_set->file_size = BLOCKSIZE;
						node_to_set->c_date = get_current_time();
						node_to_set->m_date = get_current_time();

						paquete = slz_simple_res(COD_MKNOD);
						paquete_enviar(cliente_fd, paquete);
						return;
					}
				}
			}
		}
	}

	log_msje_error("mkdir: [ %s ]", strerror(error));
	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);
	return;
}

void sac_rmdir(char *path, int cliente_fd)
{
	log_msje_info("SAC RMDIR Path = [ %s ]", path);
	package_t paquete;
	int error;

	int node_to_set = fs_get_blk_by_fullpath(path);

	if (node_to_set == -1)
	{
		error = ENOENT;
	}
	else
	{
		if(sac_nodetable[node_to_set].state != 2)
		{
			error = ENOTDIR;
		}
		else
		{
			if(!fs_is_empty_directory(node_to_set))
			{
				error = ENOTEMPTY;
			}
			else
			{
				fs_remove_dir(node_to_set);

				log_msje_info("Exito operacion rmdir sobre disco");
				paquete = slz_simple_res(COD_RMDIR);
				paquete_enviar(cliente_fd, paquete);
				return;

			}
		}

	}
	log_msje_error("rmdir: [ %s ]", strerror(error));
	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);
	return;

}


void sac_write(char *path, char *buffer, uint32_t blk, size_t size, off_t offset, int cliente_fd)
{
	log_msje_info("SAC WRITE Path = [ %s ]", path);
	package_t paquete;

	int escrito;
	escrito = fs_write_file(blk, buffer, size, offset);

    log_msje_info("Exito operacion write sobre disco");
    paquete = slz_res_write(escrito);

    paquete_enviar(cliente_fd, paquete);
}

void sac_unlink(char *path, int cliente_fd)
{
	log_msje_info("SAC UNLINK = [ %s ]", path);
	package_t paquete;
	int error;

	int node_to_set = fs_get_blk_by_fullpath(path);

	if (node_to_set == -1)
	{
		error = ENOENT;
	}
	else
	{
		if(sac_nodetable[node_to_set].state == 2)
		{
			error = EISDIR;
		}
		else
		{
			fs_delete_file(node_to_set);

			log_msje_info("Exito operacion delete sobre disco");
			paquete = slz_simple_res(COD_UNLINK);
			paquete_enviar(cliente_fd, paquete);
			return;
		}

	}
	log_msje_error("unlink: [ %s ]", strerror(error));
	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);
	return;
}


void sac_truncate(char *path, off_t newsize, int cliente_fd)
{
	log_msje_info("SAC TRUNCATE path = [ %s ]", path);
	package_t paquete;
	int error;

	int node_to_set = fs_get_blk_by_fullpath(path);

	if (node_to_set == -1)
	{
		error = ENOENT;
	}
	else
	{
		if(sac_nodetable[node_to_set].state == 2)
		{
			error = EISDIR; //Pathname refers to a directory
		}

		if(sac_nodetable[node_to_set].state == 1)
		{
			if (newsize < 0 || newsize >= fs_get_max_filesize())
			{
				error = EINVAL;//Invalid argument
			}
			else
			{
				fs_truncate_file(node_to_set, newsize);

				log_msje_info("Exito operacion truncate sobre disco");
				paquete = slz_simple_res(COD_TRUNCATE);
				paquete_enviar(cliente_fd, paquete);
				return;
			}

		}
	}

	log_msje_error("truncate: [ %s ]", strerror(error));
	paquete = slz_res_error(error);
	paquete_enviar(cliente_fd, paquete);

}

void sac_rename(char *path, char *newpath, int cliente_fd)
{
	log_msje_info("SAC RENAME path = [ %s ]", path);
	log_msje_info("to new name = [ %s ]", newpath);
	package_t paquete;
	int error;
	bool valid_to_rename = false;
	bool want_to_move_to_dir = false;
	int father_newpath;

	int node_to_set = fs_get_blk_by_fullpath(path);
	int node_new_path = fs_get_blk_by_fullpath(newpath);

	if (node_to_set == -1)
	{
		error = ENOENT;
	}
	else
	{
		char *prev_path = get_lastfile_previous_path(newpath);
		father_newpath = fs_get_blk_by_fullpath(prev_path);

		if (father_newpath == -1)
		{
			error = ENOENT; //No such file or directory of new path
		}
		else
		{
			if(node_new_path == -1)//no existe el nombre
			{
				valid_to_rename = true;

				if(sac_nodetable[father_newpath].state == 2) { want_to_move_to_dir = true; }
			}
			else //existe
			{
				if(sac_nodetable[node_to_set].state == 1)// file
				{
					if(sac_nodetable[node_new_path].state == 2)//dir
					{
						error = EISDIR;
					}
					else//both file
					{
						valid_to_rename = true;
					}

				}
				else if(sac_nodetable[node_to_set].state == 2) //dir
				{
					if(sac_nodetable[node_new_path].state == 2)//dir
					{
						if(!fs_is_empty_directory(node_new_path))
						{
							error = ENOTEMPTY;
						}
						else//empty dir
						{
							valid_to_rename = true;
						}
					}
					else
					{
						error = ENOTDIR;
					}
				}
			}

		}

	}
	if(valid_to_rename)
	{
		char* newname = get_last_filename_from_path(newpath);
		if(want_to_move_to_dir)
		{
			sac_nodetable[node_to_set].parent_dir_block = father_newpath;
		}

		fs_rename_file(node_to_set, newname);
		sac_nodetable[node_to_set].m_date = get_current_time();

		//actualizo mis nodos
		int inicio_tabla_nodos = 1 + sac_header->size_bitmap;
		memcpy(disk_blk_pointer + inicio_tabla_nodos , sac_nodetable, GFILEBYTABLE*BLOCKSIZE);

		log_msje_info("Exito operacion rename sobre disco");
		paquete = slz_simple_res(COD_RENAME);
		paquete_enviar(cliente_fd, paquete);
		return;
	}
	else
	{
		log_msje_error("rename: [ %s ]", strerror(error));
		paquete = slz_res_error(error);
		paquete_enviar(cliente_fd, paquete);
		return;
	}

}

