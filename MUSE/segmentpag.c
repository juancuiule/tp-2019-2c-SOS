#include "segmentpag.h"

t_list *tables;
t_log *logger;
t_bitarray *frame_usage_bitmap;
void* bitmap_pointer;

void init_structures(int frames) {
	tables = list_create();
	logger = log_create("./logs/segmentpag.log", "Segment", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil(frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);
}

t_page *create_page(int frame_number) {
	t_page * page = malloc(sizeof(t_page));
	page->flag = false;
	page->frame_number = frame_number;
	log_info(logger, "Nueva pagina creada");
	return page;
}

process_segment *create_segment(segment_type type, int base, int size) {
	process_segment *segment = malloc(sizeof(process_segment));
	segment->type = type;
	segment->base = base;
	segment->size = size;
	t_list* pages = list_create();
	segment->pages= pages;
	log_info(logger, "Nuevo segmento creado");
	return segment;
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	t_list* segments = list_create();
	new_table->segments = segments;
	list_add(tables, new_table);
	log_info(logger, "Se creo una process_table para el proceso: %s", process);
}

void alloc_for_process(char* process, int size) {
	process_table* process_table = get_table_for_process(process);
	int is_heap(process_segment *segment) {
		return segment->type == HEAP;
	}
	list_filter(process_table->segments, (void*) is_heap);
}

void add_process_segment(char* process, process_segment* segment) {
	process_table* process_table = get_table_for_process(process);
	list_add((*process_table).segments, segment);
}

process_table* get_table_for_process(char* process) {
	int is_this(process_table* segment_table) {
		return string_equals_ignore_case(segment_table->process, process);
	}
	t_list* filtered = list_filter(tables, (void*) is_this);
	if (filtered->elements_count > 0) {
		return filtered->head->data;
	} else {
		return NULL;
	}
}
