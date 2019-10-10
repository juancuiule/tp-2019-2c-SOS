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
