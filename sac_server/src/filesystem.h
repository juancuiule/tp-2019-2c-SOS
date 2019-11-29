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
#include <errno.h>


#define BLOCKSIZE 4096
#define GFILEBYTABLE 1024
#define GFILEBYBLOCK 1
#define GFILENAMELENGTH 71
#define GHEADERBLOCKS 1
#define BLKINDIRECT 1000
#define BLKDATA 1024
#define MAX_FILE_NUMBER 1024

typedef uint32_t ptrGBloque;

typedef struct sac_block_t{ // un bloque
	unsigned char bytes[BLOCKSIZE];
} GBlock;

typedef struct sac_blk_i { // un bloque ind simple
	ptrGBloque blk_datos[BLKDATA]; //array de 1024 blks de datos
} GBlock_IndSimple;

typedef struct sac_header_t { // un bloque header
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
void * disk_addr;
size_t disk_size;

GHeader *sac_header;
t_bitarray *sac_bitarray;
GFile *sac_nodetable;//mi tabla de nodos


typedef struct blk_candidato{
	int the_blk;
	int blk_father;
} GBlk_nominee;

int disk_fd;

//desc: Setea las estructuras que necesitamos para manipular el SAC filesystem
void fs_set_config();

//desc: Mapea en memoria el disco - archivo binario con la estructura del filesystem
bool fs_map_disk_in_memory(char *disk_name);

void fs_munmap_disk();

//desc: Obtiene el bloque de metadata del archivo. -1 si no existe el archivo.
int fs_get_blk_by_fullpath(char *fullpath);

//desc: Devuelve un blk nodo libre o EDQUOT si no hay mas nodos libres
int fs_get_free_blk_node();

//desc: Chequea que la ruta exista
bool fs_path_exist(char *path);

//desc: Obtiene todos los filenames de un directorio
void fs_get_child_filenames_of(uint32_t blk_father, t_list *filenames);

//desc: devuelve direccion de un bloque de datos libre
int get_free_blk_data_dir();

//desc: devuelve un bloque ind simple con su primer bloque de datos seteado o EDQUOT si no hay mas blks de datos
int fs_get_blk_ind_with_data_blk();

size_t fs_read_file(char *buf, size_t size, off_t offset, uint32_t node_blk);

size_t fs_write_file(uint32_t node_blk, char *buffer, size_t size, off_t offset);

void fs_truncate_file(int node, off_t newsize);

bool node_has_blk_assigned(int node, int blk_data);
bool node_has_blk_ind_assigned(int node, int blk_ind);
int fs_get_next_index_blk_indsimple_to_assign(int node);
int fs_get_next_index_blk_data_to_assign(int blk_ind);

#endif /* FILESYSTEM_H_ */
