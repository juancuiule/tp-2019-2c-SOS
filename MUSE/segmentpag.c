#include "segmentpag.h"

void init_structures(int frames) {
	tables = list_create();
	logger = log_create("./logs/segmentpag.log", "Segment", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil((double) frames / 8);
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

void add_page_to_segment(process_segment* segment, t_page* page) {
	list_add(segment->pages, page);
}

process_segment *find_segment_with_space(t_list* segments, int size) {
	int has_space(process_segment* segment) {
		// TODO: calcular si el segmento tiene espacio;
		return 0;
	}
	return list_find(segments, (void*) has_space);
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	t_list* segments = list_create();
	new_table->segments = segments;
	list_add(tables, new_table);
	log_info(logger, "Se creo una process_table para el proceso: %s", process);
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

int find_free_frame(t_bitarray* bitmap) {
	int var;
	for (var = 0; var < bitmap->size; ++var) {
		bool is_used = bitarray_test_bit(bitmap, var);
		if (!is_used) {
			return var;
		}
	}
	return -1;
}
