#include "segmentpag.h"

void init_structures(int m_size, int p_size) {
	int frames = m_size / p_size;
	tables = list_create();
	seg_logger = log_create("./logs/segmentpag.log", "segmentpag", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil((double) frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);

	MEMORY = calloc(frames, p_size);
	int i;
	for(i = 0; i < frames; i++) {
		void* new_frame = malloc(p_size);
		bool is_free = true;
		uint32_t size = p_size - sizeof(bool) - sizeof(uint32_t);
		memcpy(new_frame, &is_free, sizeof(bool));
		memcpy(new_frame + sizeof(bool), &size, sizeof(uint32_t));
		*(MEMORY + i) = new_frame;
		// free(new_frame);
	}
}

t_page *create_page(int frame_number) {
	if (bitarray_test_bit(frame_usage_bitmap, frame_number)) {
		log_error(seg_logger, "Se está queriendo crear una página con el frame: %i que ya está en uso", frame_number);
		return NULL; // explotar acá!
	} else {
		t_page * page = malloc(sizeof(t_page));
		page->flag = true;
		page->frame_number = frame_number;
		page->in_use = 1;
		page->modified = 0;
		log_debug(seg_logger, "Nueva pagina creada relacionada con el frame %i", frame_number);
		bitarray_set_bit(frame_usage_bitmap, frame_number);
		return page;
	}
}

process_segment *create_segment(segment_type type, uint32_t base, int size) {
	process_segment *segment = malloc(sizeof(process_segment));
	segment->type = type;
	segment->base = base;
	segment->size = size;
	t_list* pages = list_create();
	segment->pages= pages;
	log_debug(seg_logger, "Nuevo segmento (%s) creado. base: %i, size: %i", type == HEAP ? "HEAP" : "MMAP", base, size);
	return segment;
}

void add_page_to_segment(process_segment* segment, t_page* page) {
	list_add((*segment).pages, page);
	log_info(seg_logger, "segment has: %i pages", segment->pages->elements_count);
}

process_segment *find_segment_with_space(t_list* segments, int size) {
	int is_heap(process_segment* segment) {
		return segment->type == HEAP;
	};

	t_list* heap_segments = list_filter(segments, (void*) is_heap);

	int has_space(process_segment* segment) {
		return find_page_with_space(segment->pages, size) != NULL;
	}
	return list_find(heap_segments, (void*) has_space);
}

t_page* find_page_with_space(t_list* pages, int size) {
	int has_free_frame(t_page* page) {
		void* frame = MEMORY[page->frame_number];
		int space = get_frame_free_size(frame);
		return space > size;
	}
	return list_find(pages, (void*) has_free_frame);
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	t_list* segments = list_create();
	new_table->segments = segments;
	list_add(tables, new_table);
	log_info(seg_logger, "Se creo una process_table para el proceso: %s", process);
}

void add_process_segment(char* process, process_segment* segment) {
	process_table* process_table = get_table_for_process(process);
	list_add((*process_table).segments, segment);
	log_info(seg_logger, "process has: %i segments", process_table->segments->elements_count);

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

void* alloc_in_frame(int frame_number, uint32_t size) {
	void* frame = MEMORY[frame_number];
	void* pointer = NULL;
	bool x = false;
	bool y = true;

	bool is_free = false;
	uint32_t data_size = 0;
    uint32_t offset = 0;

    // Me muevo hasta encontrar un espacio libre o que se termine el frame...
	while (offset < PAGE_SIZE) {
		log_info(seg_logger, "is_free: %i, data_size: %u, offset: %i", is_free, data_size, offset);
		memcpy(&is_free, frame + offset, sizeof(bool));
		log_info(seg_logger, "is_free: %i", is_free);
		if (is_free) {
			break;
		}
		offset += sizeof(bool);
		memcpy(&data_size, frame + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		log_info(seg_logger, "data_size: %i", data_size);
		offset += data_size;
	}

	// Si queda espacio libre entonces lo ocupo
	// y al final agrego mas metadata para lo que queda free
	if (is_free) {
		// log_info(seg_logger, "is_free: %i, data_size: %u, offset: %u", is_free, data_size, offset)
		memcpy(frame + offset, &x, sizeof(bool));
		offset += sizeof(bool);
		memcpy(frame + offset, &(size), sizeof(uint32_t));
		offset += sizeof(uint32_t);
		pointer = offset; // acá empieza la dirección de datos de lo que se pidio
		offset += size;
		memcpy(frame + offset, &y, sizeof(bool));
		offset += sizeof(bool);
		uint32_t remaining_size = PAGE_SIZE - offset - sizeof(uint32_t);
		log_info(seg_logger, "remaining_size %i", remaining_size);
		memcpy(frame + offset, &(remaining_size), sizeof(uint32_t));
	}
	return pointer;
}

int get_frame_free_size(void* frame) {
	log_debug(seg_logger, "get_frame_free_size");
	bool is_free = 0;
	uint32_t data_size = 0;
	uint32_t offset = 0;

	// Me muevo hasta encontrar un espacio libre o que se termine el frame...
	while (offset < PAGE_SIZE) {
		memcpy(&is_free, frame + offset, sizeof(bool));
		offset += sizeof(bool);
		memcpy(&data_size, frame + offset, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		offset += data_size;
		if (is_free) {
			return data_size;
		}
		data_size = 0;
	}
	return 0;
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
