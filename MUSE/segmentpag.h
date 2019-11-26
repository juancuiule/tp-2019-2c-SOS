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
t_log *logger;
t_bitarray *frame_usage_bitmap;
void* bitmap_pointer;
void** MEMORY;
int MEMORY_SIZE;
int PAGE_SIZE;
int SWAP_SIZE;

typedef enum {
	HEAP = 0,
	MMAP = 1
} segment_type;

typedef struct {
	t_list *segments; // lista de segmentos
	char *process; // a que proceso pertenece
} process_table;

typedef struct {
	segment_type type; // segmento de heap o mmap
	uint32_t base; // base
	uint32_t size; // tamaño
	t_list *pages; // lista de paginas
} process_segment;

typedef struct {
	int frame_number; // frame de memoria o posición de swap
	bool in_use; // para clock modificado
	bool modified; // para clock modificado
	bool flag; // bit de presencia
} t_page;

//typedef struct {
//	uint32_t size;
//	bool is_free;
//} frame_metadata;
//
//typedef struct {
//	frame_metadata* metadata;
//	void* data;
//} frame_block;
//
//typedef struct {
//	int frame_number;
//	t_list* blocks; // frame_block(s)
//} frame;

void init_structures();
t_page *create_page(int frame_number);
process_segment *create_segment(segment_type type, uint32_t base, int size);
void create_process_table(char* process);
process_table* get_table_for_process(char* process);
void add_process_segment(char* process, process_segment* segment);
void add_page_to_segment(process_segment* segment, t_page* page);
int find_free_frame(t_bitarray* bitmap);
void register_used_space_in_frame(int frame_number, uint32_t size);

#endif
