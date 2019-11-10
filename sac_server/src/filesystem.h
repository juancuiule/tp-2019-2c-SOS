#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>//open
#include <common/log.h>
#include <commons/bitarray.h>
#include <commons/string.h>
#include <commons/collections/list.h>
#include "utils.h"

#define BLOCKSIZE 4096
#define GFILEBYTABLE 1024
#define GFILEBYBLOCK 1
#define GFILENAMELENGTH 71
#define GHEADERBLOCKS 1
#define BLKINDIRECT 1000

#define MAX_FILE_NUMBER 1024

typedef uint32_t ptrGBloque;

typedef struct sac_block_t{
	unsigned char bytes[BLOCKSIZE];
} GBlock;

typedef struct sac_header_t { // un bloque
	unsigned char sac[3];
	uint32_t version;
	uint32_t blk_bitmap;
	uint32_t size_bitmap; // en bloques
	unsigned char padding[4081];
} GHeader;

typedef struct sac_file_t { // un cuarto de bloque (256 bytes)
	uint8_t state; // 0: borrado, 1: archivo, 2: directorio
	char fname[GFILENAMELENGTH];
	uint32_t parent_dir_block;
	uint32_t file_size;
	uint64_t c_date;
	uint64_t m_date;
	ptrGBloque blk_indirect[BLKINDIRECT];
} GFile;


GBlock* disk_blk_pointer;
void* disk;
size_t disk_size;

GHeader *sac_header;
t_bitarray *sac_bitarray;
GFile *sac_nodetable;//mi tabla de nodos

typedef struct blk_candidato{
	int the_blk;
	int blk_father;
} GBlk_nominee;


void fs_set_config();
bool fs_map_disk_in_memory(char *disk_name);
int fs_get_blk_by_fullpath(char *fullpath);

#endif /* FILESYSTEM_H_ */
