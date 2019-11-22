#ifndef SEGMENTPAG_H
#define SEGMENTPAG_H

#include <stdlib.h>
#include <stdbool.h>
#include <commons/collections/list.h>
#include <commons/string.h>
#include <commons/log.h>
#include <stdint.h>

typedef enum {
	HEAP,
	MMAP
} segment_type;

typedef struct {
	t_list *segments; // lista de segmentos
	char *process; // a que proceso pertenece
} process_table;

typedef struct {
	segment_type type; // segmento de heap o mmap
	int base; // base
	int size; // tamaño
	t_list *pages; // lista de paginas
} t_segment;

typedef struct {
	int frame_number; // frame de memoria o posición de swap
	bool flag; // bit de presencia
	char **reg;
} t_page;

void init_structures();
t_page *create_page(int frame_number, char **reg);
t_segment *create_segment(segment_type type, int base, int size);
void create_process_table(char* process);
process_table* get_table_for_process(char* process);
void add_process_segment(char* process, t_segment* segment);

#endif
