#include "segmentpag.h"

void init_structures(int m_size, int p_size) {
	int frames = m_size / p_size;
	tables = list_create();
	logger = log_create("./logs/segmentpag.log", "Segment", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil((double) frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);

	MEMORY = malloc(m_size);
//	int i;
//	for(i = 0; i < frames; i++) {
//		frame* new_frame = malloc(sizeof(frame));
//		new_frame->frame_number = i;
//		new_frame->blocks = list_create();
//		frame_metadata* metadata = malloc(sizeof(frame_metadata));
//		log_info(logger, "frame metadata size: %i", sizeof(frame_metadata));
//		metadata->is_free = 1;
//		metadata->size = p_size - sizeof(frame_metadata);
//		// log_info(logger, "frame metadata size: %i", metadata->size);
//		frame_block* block = malloc(sizeof(frame_block));
//		block->data = NULL;
//		block->metadata = metadata;
//		memcpy(MEMORY, new_frame + i * sizeof(frame), sizeof(frame) * i);
//	}
}

t_page *create_page(int frame_number) {
	t_page * page = malloc(sizeof(t_page));
	page->flag = false;
	page->frame_number = frame_number;
	page->in_use = 1;
	page->modified = 0;
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
	int is_heap(process_segment* segment) {
		return segment->type == HEAP;
	};
	t_list* heap_segments = list_filter(segments, (void*) is_heap);
	log_info(logger, "Heap Segments Length %i", heap_segments->elements_count);

	int has_space(process_segment* segment) {
		int has_free_frame(t_page* page) {
			frame* a_frame = malloc(sizeof(frame));
			log_info(logger, "frame number %i", page->frame_number);
			memcpy(a_frame, MEMORY + page->frame_number * sizeof(frame), sizeof(frame));

			log_info(logger, "frame number %i, blocks: %i", a_frame->frame_number, a_frame->blocks->elements_count);
			int frame_with_space(frame_block* block) {
				log_info(logger, "data %i, size: %i", block->data, block->metadata->size);
				return block->metadata->is_free && block->metadata->size > size;
			}
			return list_find(a_frame->blocks, (void*) frame_with_space);
		}
		return list_any_satisfy(segment->pages, (void*) has_free_frame);
	}
	return list_find(heap_segments, (void*) has_space);
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
