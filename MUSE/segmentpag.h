#ifndef SEGMENTPAG_H
#define SEGMENTPAG_H

#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/bitarray.h>
#include <math.h>
#include <stdint.h>

t_list *tables;
t_log *seg_logger;
t_bitarray *frame_usage_bitmap;
void* bitmap_pointer;
void** MEMORY;
int MEMORY_SIZE;
int PAGE_SIZE;
int SWAP_SIZE;
FILE *swap_file;
int metadata_size = sizeof(bool) + sizeof(uint32_t);

typedef enum {
	HEAP = 0,
	MMAP = 1
} segment_type;

typedef struct {
	void *segments; // segmentos
	int number_of_segments;
	char *process; // a que proceso pertenece
} process_table;

typedef struct {
	segment_type type; // segmento de heap o mmap
	uint32_t base; // base
	uint32_t size; // tamaño
	void *pages; // paginas
} process_segment;

typedef struct {
	int frame_number; // frame de memoria o posición de swap
	bool in_use; // para clock modificado
	bool modified; // para clock modificado
	bool flag; // bit de presencia
} t_page;

int metadata_size;

//typedef struct {
//	uint32_t size;
//	bool is_free;
//} frame_metadata;

void init_structures();
t_page *create_page(int frame_number);
process_segment *create_segment(segment_type type, uint32_t base);
void create_process_table(char* process);
process_table* get_table_for_process(char* process);
void add_process_segment(char* process, process_segment* segment);
void add_page_to_segment(process_segment* segment, t_page* page);
int find_free_frame(t_bitarray* bitmap);
process_segment* find_segment_with_space(process_table* table, int size);
process_segment* segment_by_dir(process_table* table, int dir);
process_segment* find_extensible_heap_segment(process_table* table);
int free_space_at_the_end(process_segment* segment);
void clear_bitmap(int bits);
void cpy_to_dir(process_segment* segment, uint32_t dir, void* val, int size);
void free_dir(process_segment* segment, uint32_t dir);


void* get_from_dir(process_segment* segment, uint32_t dir, int size);

uint32_t alloc_in_segment(process_segment* segment, uint32_t dir, uint32_t size);
void* find_free_dir(process_segment* segment, int size);
void* set_in_segment(process_segment* segment, uint32_t dir, uint32_t size, void* value);

void* get_metadata_from_segment(process_segment* segment, uint32_t dir, bool* is_free, uint32_t* size);
void* get_from_segment(process_segment* segment, uint32_t dir, uint32_t size, void* to);

void* set_metadata_in_segment(process_segment* segment, uint32_t dir, bool is_free, uint32_t size);

#endif
