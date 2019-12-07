#ifndef SAC_OP_H_
#define SAC_OP_H_

#include <stdlib.h>
#include <dirent.h>//systypes.h
#include <string.h>
#include <errno.h>
#include <common/log.h>
#include <common/protocol.h>
#include <fcntl.h>//open
#include <sys/stat.h>//lstat
#include <unistd.h>//read pread
#include <commons/collections/list.h>
#include <sys/types.h>

#include "filesystem.h"
#include <stdint.h>


#define FS_PATH "/home/utnso/workspace/tp-2019-2c-SOS/sac_server/Debug/SAC"
#define PATH_MAX 4096 //256

void sac_opendir(char *path, int cliente_fd);
void sac_readdir(char *path, uint32_t blk_number, int cliente_fd);
void sac_releasedir(char *path, intptr_t dir, int cliente_fd);
void sac_open(char *path, int flags, int cliente_fd);
void sac_getattr(char *path, int cliente_fd);
void sac_read(char *path, uint32_t blk, size_t size, off_t offset, int cliente_fd);
void sac_release(char *path, int fd, int cliente_fd);
void sac_mkdir(char *path, int cliente_fd);
void sac_rmdir(char *path, int cliente_fd);
void sac_mknod(char *path, int cliente_fd);
void sac_write(char *path, char *buffer, uint32_t blk, size_t size, off_t offset, int cliente_fd);
void sac_unlink(char *path, int cliente_fd);
void sac_truncate(char *path, off_t newsize, int cliente_fd);
void sac_rename(char *path, char* newpath, int cliente_fd);

#endif /* SAC_OP_H_ */
