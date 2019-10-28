#ifndef CLI_OP_H_
#define CLI_OP_H_

#include <stdio.h>
#include <fuse.h>
#include <string.h>
#include <errno.h>
#include <common/log.h>
#include <fcntl.h>
#include <stddef.h>

#include <common/protocol.h>
#include <commons/collections/list.h>

socket_t sac_server;

int cli_getattr(const char *path, struct stat *stbuf);
int cli_opendir(const char *path, struct fuse_file_info *fi);
int cli_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi);
int cli_releasedir(const char *path, struct fuse_file_info *fi);
int cli_open(const char *path, struct fuse_file_info *fi);
int cli_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int cli_flush(const char *path, struct fuse_file_info *fi);
int cli_release(const char *path, struct fuse_file_info *fi);
int cli_mkdir(const char *path, mode_t mode);
int cli_rmdir(const char *path);

void set_sac_fd(socket_t socket);

struct fuse_operations cli_oper;

#endif /* CLI_OP_H_ */
