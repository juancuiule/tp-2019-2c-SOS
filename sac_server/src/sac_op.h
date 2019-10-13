#ifndef SAC_OP_H_
#define SAC_OP_H_

#include <stdlib.h>
#include <dirent.h>//systypes.h
#include <string.h>
#include <errno.h>
#include <common/log.h>
#include <common/protocol.h>

#include <commons/collections/list.h>

#define FS_PATH "/home/utnso/workspace/tp-2019-2c-SOS/sac_server/Debug/SAC"
#define PATH_MAX 4096 //256

void sac_opendir(char *path, int cliente_fd);
void sac_readdir(char *path, intptr_t dir, int cliente_fd);
void sac_releasedir(char *path, intptr_t dir, int cliente_fd);

#endif /* SAC_OP_H_ */
