#ifndef COMMON_PROTOCOL_H_
#define COMMON_PROTOCOL_H_

#include <stdlib.h>
#include "log.h"
#include <string.h>
#include <errno.h>
#include <sys/socket.h>
#include "socket.h"
#include <dirent.h>

#include <commons/collections/list.h>

#define TAM_HEADER 9

typedef enum{
    COD_HANDSHAKE,
	COD_OPENDIR,
	COD_READDIR,
	COD_RELEASEDIR,
	COD_OPEN,
	COD_GETATTR,
	COD_ERROR
}cod_operation;

typedef struct{
    char cod_proceso;
    int cod_operacion;
    int tam_payload;
}__attribute__((packed))
header_t;

typedef struct{
	header_t header;
    void * payload;
}package_t;

typedef struct{
    int socket;
}pthread_data;

void handshake_recibir(int socket);
void handshake_enviar(int socket, char cod_proc);

package_t paquete_recibir(int socket);
bool paquete_enviar(int socket, package_t paquete);


//---Protocolo operaciones sac cli---

package_t slz_cod_readdir(const char *path, intptr_t dir);
package_t slz_cod_opendir(const char *path);
package_t slz_cod_releasedir(const char *path, intptr_t dir);
package_t slz_cod_open(const char *path, int flags);
package_t slz_cod_getattr(const char *path);

void dslz_res_opendir(void *buffer, intptr_t* dir);
void dslz_res_readdir(void *buffer, t_list** filenames);
//res_closedir ... no recibimos nada en el payload
void dslz_res_open(void *buffer, int *fd);
void dslz_res_getattr(void *buffer, uint32_t *mode, uint32_t *nlink);

//---Protocolo operaciones sac server---

void dslz_cod_readdir(void *buffer, char**path, intptr_t *dir);
void dslz_cod_opendir(void *buffer, char**path);
void dslz_cod_releasedir(void* buffer, char** path, intptr_t* dir);
void dslz_cod_open(void *buffer, char **path, int *flags);
void dslz_cod_getattr(void *buffer, char**path);

package_t slz_res_opendir(DIR *dp, bool error);
package_t slz_res_readdir(t_list * filenames, bool error);
package_t slz_res_releasedir(bool error);
package_t slz_res_open(int fd, bool error);
package_t slz_res_getattr(uint32_t mode, uint32_t nlink, bool error);

#endif /* COMMON_PROTOCOL_H_ */
