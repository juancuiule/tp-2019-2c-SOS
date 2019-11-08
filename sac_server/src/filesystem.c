#include "filesystem.h"

//El header se encuentra en el primer bloque de mi disco
static void set_sac_header()
{
	sac_header = (GHeader*) disk_addr;

	log_msje_info("Name : [ %s ]", sac_header->sac);
	log_msje_info("Version : [ %u ]", sac_header->version);
	log_msje_info("Bloque de bitmap: [ %u ]", sac_header->blk_bitmap);
	log_msje_info("Bitmap bloques : [ %u ]", sac_header->size_bitmap);
}

void filesystem_config()
{
	set_sac_header();
	//set_bitmap();
	//set_nodetable();
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
	size_t disk_size = get_filesize(disk_name);
	int disk_fd = open(disk_name, O_RDWR, 0);
	disk = mmap(NULL, disk_size, PROT_READ | PROT_WRITE, MAP_FILE | MAP_SHARED, disk_fd, 0);
	disk_addr = disk;
	return disk != MAP_FAILED;
}
