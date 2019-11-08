#include "filesystem.h"

//El header se encuentra en el primer bloque de mi disco
static void set_sac_header()
{
	sac_header = (GHeader*) disk_blk_pointer;
	log_msje_info("Name : [ %s ]", sac_header->sac);
	log_msje_info("Version : [ %u ]", sac_header->version);
	log_msje_info("Bloque de bitmap: [ %u ]", sac_header->blk_bitmap);
	log_msje_info("Bitmap bloques : [ %u ]", sac_header->size_bitmap);
}

static void set_sac_bitmap()
{
	int bitmap_blk = sac_header->blk_bitmap;
	int bitmap_size_bytes = disk_size / BLOCKSIZE / 8;
	log_msje_info("Bitmap size in bytes is [ %d ]", bitmap_size_bytes);
	sac_bitarray = bitarray_create_with_mode((char *)disk_blk_pointer + bitmap_blk, bitmap_size_bytes, LSB_FIRST);
}


static void set_sac_nodetable()
{
	GBlock* node_blk_start = disk_blk_pointer + 1 + sac_header->size_bitmap;
	sac_nodetable = (GFile*)node_blk_start;
	log_msje_info("Nodetable seteado");
}

void filesystem_config()
{
	log_msje_info("Seteo estructuras del filesystem");
	set_sac_header();
	set_sac_bitmap();
	set_sac_nodetable();
}

static size_t get_filesize(char *filename)
{
	FILE* fd = fopen(filename, "r");

	fseek(fd, 0L, SEEK_END);
	size_t size = ftell(fd);

	fclose(fd);
	return size;
}

bool map_disk_in_memory(char *disk_name)
{
	disk_size = get_filesize(disk_name);
	int disk_fd = open(disk_name, O_RDWR, 0);
	disk = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, disk_fd, 0);
	disk_blk_pointer = disk;
	return disk != MAP_FAILED;
}
