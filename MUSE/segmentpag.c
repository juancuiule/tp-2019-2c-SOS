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
	segment->pages= NULL;
	log_debug(seg_logger, "Nuevo segmento (%s) creado. base: %i, size: %i", type == HEAP ? "HEAP" : "MMAP", base, size);
	return segment;
}

void add_page_to_segment(process_segment* segment, t_page* page) {
	void* pages = segment->pages;
	int number_of_pages = segment->size / PAGE_SIZE;
	if (pages == NULL) {
		// first page
		segment->pages = malloc(sizeof(t_page));
		memcpy(segment->pages, page, sizeof(t_page));
	} else {
		log_info(seg_logger, "el segmento de base: %i. tiene %i paginas", segment->base, number_of_pages);
		int offset = number_of_pages * sizeof(t_page);
		realloc(segment->pages, pages + offset + sizeof(t_page));
		memcpy(segment->pages + offset, page, sizeof(t_page));
		segment->size += PAGE_SIZE;
	}
	log_info(seg_logger, "segment has: %i pages", segment->size / PAGE_SIZE);
}

process_segment* find_segment_with_space(process_table* table , int size) {
	int is_heap(process_segment* segment) {
		return segment->type == HEAP;
	};
	int has_space(process_segment* segment) {
		return find_page_with_space(segment, size) != NULL;
	}

	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	log_info(seg_logger, "%s tiene %i segments", table->process, table->number_of_segments);
	int i = 0;
	while (i < table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		if (is_heap(segment) && has_space(segment)) {
			return table->segments + i * sizeof(process_segment);
		}
		i++;
	}
	return NULL;
}

process_segment* segment_by_dir(process_table* table, int dir) {
	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		if (segment->base < dir && (segment->base + segment->size) > dir) {
			return segment;
		}
		i++;
	}
	free(segment);
	return NULL;
}

process_segment* find_extensible_heap_segment(process_table* table) {
	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int offset = (table->number_of_segments - 1) * sizeof(process_segment);
	memcpy(segment, segments + offset, sizeof(process_segment));

	log_info(
		seg_logger,
		"last seg, base: %i, fin: %i, type: %s",
		segment->base,
		segment->base + segment->size,
		segment->type == HEAP ? "HEAP" : "MMAP"
	);
	if (segment->type == HEAP) {
		return segment;
	} else {
		return NULL;
	}
}

int last_position(char* process) {
	process_table* process_table = get_table_for_process(process);
	void* segments = process_table->segments;
	int i = 0;
	int last = 0;
	process_segment* segment = malloc(sizeof(process_segment));
	while (i < process_table->number_of_segments) {
		memcpy(segment, segments + i * sizeof(process_segment), sizeof(process_segment));
		last += segment->size;
		i++;
	}
	free(segment);
	return last;
}

t_page* find_page_with_space(process_segment* segment, int size) {
	void* pages = segment->pages;
	int number_of_pages = segment->size / PAGE_SIZE;
	log_debug(seg_logger, "find_page_with_space, number_of_pages: %i", number_of_pages);

	int has_free_frame(t_page* page) {
		void* frame = MEMORY[page->frame_number];
		int space = get_frame_free_size(frame);
		return space > size;
	}

	t_page* page = malloc(sizeof(t_page));
	int i = 0;
	while (i < number_of_pages) {
		log_info(seg_logger, "i: %i", i);
		memcpy(page, pages + i * sizeof(t_page), sizeof(t_page));
		if (has_free_frame(page)) {
			return page;
		}
		i++;
	}
	free(page);
	return NULL;
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	new_table->segments = NULL;
	new_table->number_of_segments = 0;
	list_add(tables, new_table);
	log_info(seg_logger, "Se creo una process_table para el proceso: %s", process);
}

void add_process_segment(char* process, process_segment* segment) {
	process_table* process_table = get_table_for_process(process);
	void* segments = process_table->segments;
	if (segments == NULL) {
		// first segment
		process_table->segments = malloc(sizeof(process_segment));
		memcpy(process_table->segments, segment, sizeof(process_segment));
	} else {
		log_info(seg_logger, "pre add... %s tiene %i segments", process, process_table->number_of_segments);
		int offset = process_table->number_of_segments * sizeof(process_segment);
		realloc(process_table->segments, process_table->segments + offset + sizeof(process_segment));
		memcpy(process_table->segments + offset, segment, sizeof(process_segment));
	}
	process_table->number_of_segments += 1;
	log_info(seg_logger, "after add ... %s tiene %i segments", process, process_table->number_of_segments);
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
			log_debug(seg_logger, "get_frame_free_size, retorna %i", data_size);
			return data_size;
		}
		data_size = 0;
	}
	log_debug(seg_logger, "get_frame_free_size, retorna %i", 0);
	return 0;
}

int find_free_frame(t_bitarray* bitmap) {
	int var;
	//	log_info(seg_logger, "bitmap: %i", bitmap->bitarray);
	for (var = 0; var < (MEMORY_SIZE / PAGE_SIZE); ++var) {
		bool is_used = bitarray_test_bit(bitmap, var);
		log_info(seg_logger, "frame: %i, bit: %i, bitmap_size: %i", var, is_used, bitmap->size);
		if (!is_used) {
			return var;
		}
	}
	return -1;
}
