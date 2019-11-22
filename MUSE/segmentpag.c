#include "segmentpag.h"

t_list *tables;
t_log *logger;

void init_structures() {
	tables = list_create();
	logger = log_create("./logs/segmentpag.log", "Segment", 1, LOG_LEVEL_DEBUG);
}

t_page *create_page(int frame_number, char **reg) {
	t_page * page = malloc(sizeof(t_page));
	page->flag = false;
	page->frame_number = frame_number;
	page->reg = reg;
	log_info(logger, "Nueva pagina creada");
	return page;
}

t_segment *create_segment(segment_type type, int base, int size) {
	t_segment *segment = malloc(sizeof(t_segment));
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

void add_process_segment(char* process, t_segment* segment) {
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
