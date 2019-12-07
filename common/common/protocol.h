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
	COD_OPEN,
	COD_GETATTR,
	COD_READ,
	COD_MKDIR,
	COD_RMDIR,
	COD_MKNOD,
	COD_WRITE,
	COD_UNLINK,
	COD_ERROR,
	COD_TRUNCATE,
	COD_RENAME,
	COD_DESC
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

package_t slz_cod_readdir(const char *path, uint32_t dir);
package_t slz_cod_releasedir(const char *path, intptr_t dir);
package_t slz_cod_open(const char *path, int flags);
package_t slz_cod_read(const char *path, uint32_t blk_num, size_t size, off_t offset);
package_t slz_cod_release(const char *path, int fd);
package_t slz_cod_write(const char *path, const char *buffer, uint32_t blk, size_t size, off_t offset);
package_t slz_cod_truncate(const char *path, off_t newsize);
package_t slz_cod_rename(const char *path, const char *newpath);

void dslz_res_opendir(void *buffer, uint32_t* dir);
void dslz_res_readdir(void *buffer, t_list** filenames);
void dslz_res_open(void *buffer, uint32_t *blk_num);
void dslz_res_getattr(void *buffer, uint32_t *size, uint64_t *m_date, uint8_t *state);
void dslz_res_read(void *buffer, char *buf, int *size);
void dslz_res_write(void *buffer, int *size);

//---Protocolo operaciones sac server---

void dslz_cod_readdir(void *buffer, char**path, uint32_t *dir);
void dslz_cod_releasedir(void* buffer, char** path, uint32_t* dir);
void dslz_cod_open(void *buffer, char **path, int *flags);
void dslz_cod_read(void *buffer, char **path, uint32_t *blk, size_t *size, off_t *offset);
void dslz_cod_release(void *buffer, char **path, int *fd);
void dslz_cod_write(void *payload, char **path, char **buffer, uint32_t *fd, size_t *size, off_t *offset);
void dslz_cod_truncate(void *payload, char **path, off_t *newsize);
void dslz_cod_rename(void *payload, char **path, char **newpath);

package_t slz_res_opendir(uint32_t blk_number);
package_t slz_res_readdir(t_list * filenames);
package_t slz_res_open(uint32_t blk_num);
package_t slz_res_getattr(uint32_t size, uint64_t m_date, uint8_t state);
package_t slz_res_read(char *buf, ssize_t ssize);
package_t slz_res_write(int size);

//Para los errores
void dslz_res_error(void *buffer, int *errnum);
package_t slz_res_error(int errnum);

//Res simple
package_t slz_simple_res(cod_operation cod);

//Para las ops que envian solo path
package_t slz_path_with_cod(const char *path, cod_operation cod);
void dslz_payload_with_path(void *buffer, char**path);


#endif /* COMMON_PROTOCOL_H_ */
