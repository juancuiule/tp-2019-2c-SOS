#include "segmentpag.h"

bool t = true;
bool f = false;

int metadata_size = sizeof(bool) + sizeof(uint32_t);

void init_structures(int m_size, int p_size) {
	int frames = m_size / p_size;
	tables = list_create();
	seg_logger = log_create("./logs/segmentpag.log", "segmentpag", 1, LOG_LEVEL_DEBUG);
	int bitmap_size_in_bytes = ceil((double) frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);

	clear_bitmap(frames);

	MEMORY = calloc(frames, p_size);
	int i;
	for(i = 0; i < frames; i++) {
		void* new_frame = malloc(p_size);
		memset(new_frame, 1, sizeof(bool));
		memset(new_frame + sizeof(bool), NULL, p_size - sizeof(bool));
//		bool is_free = true;
//		uint32_t size = p_size - metadata_size;
//		memcpy(new_frame, &is_free, sizeof(bool));
//		memcpy(new_frame + sizeof(bool), &size, sizeof(uint32_t));
		*(MEMORY + i) = new_frame;
	}
}

void clear_bitmap(int bits) {
	for (int var = 0; var < bits; var++) {
		bitarray_clean_bit(frame_usage_bitmap, var);
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

process_segment *create_segment(segment_type type, uint32_t base) {
	process_segment *segment = malloc(sizeof(process_segment));
	segment->type = type;
	segment->base = base;
	segment->size = 0;
	segment->pages= NULL;
	log_debug(seg_logger, "Nuevo segmento (%s) creado. base: %i, size: %i", type == HEAP ? "HEAP" : "MMAP", base, 0);
	return segment;
}

void add_page_to_segment(process_segment* segment, t_page* page) {
	int number_of_pages = segment->size / PAGE_SIZE;
	int offset = number_of_pages * sizeof(t_page);

	if (number_of_pages == 0) {
		segment->pages = malloc(sizeof(t_page));
	} else {
		int new_size = offset + sizeof(t_page);
		segment->pages = realloc(segment->pages, new_size);
	}

	memcpy(segment->pages + offset, page, sizeof(t_page));
	segment->size += PAGE_SIZE;

	for (int var = 0; var < number_of_pages + 1; ++var) {
		t_page* the_page = malloc(sizeof(t_page));
		memcpy(the_page, segment->pages + var * sizeof(t_page), sizeof(t_page));
//		log_debug(seg_logger, "Page added frame: %i", the_page->frame_number);
	}

	log_debug(seg_logger, "Se agrego una página al segmento con base: %i, nuevo size: %i", segment->base, segment->size);
}

void print_process(process_table* table) {
	log_info(seg_logger, "Proceso:");
	log_info(seg_logger, "cantidad de segmentos: %i", table->number_of_segments);
	log_info(seg_logger, "process id: %s", table->process);
	process_segment* seg = malloc(sizeof(process_segment));
	for (int var = 0; var < table->number_of_segments; ++var) {
		memcpy(seg, table->segments + var * sizeof(process_segment), sizeof(process_segment));
		print_segment(seg);
	}
	free(seg);
}

void print_segment(process_segment* segment) {
	log_info(seg_logger, "Segmento:");
	log_info(seg_logger, "base: %i", segment->base);
	log_info(seg_logger, "size: %i", segment->size);
	log_info(seg_logger, "type: %s", segment->type == HEAP ? "HEAP" : "MMAP");

	int read = 0;
	while (read < segment->size) {
		bool is_free;
		uint32_t size;
		read = get_metadata_from_segment(segment, read, &is_free, &size);
		log_info(seg_logger, "dir: %i, is_free: %i, size: %i", read, is_free, size);
		read += size;
	}
}

void free_dir(process_segment* segment, uint32_t dir) {
	int number_of_pages = segment->size / PAGE_SIZE;
	int metadata_dir = dir - metadata_size;

	int page_number = (int) floor((double) metadata_dir / PAGE_SIZE);
	int offset_in_frame = metadata_dir - page_number * PAGE_SIZE;

	t_page* page = malloc(sizeof(t_page));

	memcpy(page, segment->pages + page_number * sizeof(t_page), sizeof(t_page));
	void* frame = MEMORY[page->frame_number];

	bool is_free;
	uint32_t data_size;

	memcpy(&is_free, frame + offset_in_frame, sizeof(bool));
	offset_in_frame += sizeof(bool);

	if (is_free) {
		log_error(seg_logger, "Ya estaba sin asignar");
	} else {
		memcpy(&data_size, frame + offset_in_frame, sizeof(uint32_t));
		offset_in_frame += sizeof(uint32_t);

		offset_in_frame -= metadata_size;

		// free
		memcpy(frame + offset_in_frame, &t, sizeof(bool));
		offset_in_frame += sizeof(bool);
		// falta dejar el frame libre...
		// bitarray_clean_bit(frame_usage_bitmap, page->frame_number);
	}

}

void* get_from_dir(process_segment* segment, uint32_t dir, int size) {
	int dir_de_pagina = dir - segment->base;
	int metadata_dir = dir_de_pagina - metadata_size;

	bool is_free;
	uint32_t data_size;

	uint32_t data_dir = get_metadata_from_segment(segment, metadata_dir, &is_free, &data_size);

	void** data = malloc(size);

	if (is_free) {
		log_error(seg_logger, "No hay un malloc hecho... se quieren traer datos de espacio no asignado");
	} else if (data_size < size) {
		log_error(seg_logger, "Se quiere traer algo que excede el espacio asignado");
	} else {
		get_from_segment(segment, data_dir, size, data);
		log_error(seg_logger, "hace el get a data_dir: %i, size: %i, data: %i", data_dir, size, *data);
	}
	return *data;
}

void cpy_to_dir(process_segment* segment, uint32_t dir, void* val, int size) {
	// "retrocedo" para poder ver si esta libre y cuanto espacio tiene ese bloque
	int metadata_dir = dir - metadata_size;
	log_info(seg_logger, "metadata_dir: %i", metadata_dir);


	// Hay que chequear si la dir es valida.
	bool is_free;
	uint32_t data_size;

	uint32_t data_dir = get_metadata_from_segment(segment, metadata_dir, &is_free, &data_size);

	if (is_free) {
		log_error(seg_logger, "No hay un malloc hecho...");
	} else if (data_size < size) {
		log_error(seg_logger, "No hay espacio suficiente...");
	} else {
		log_info(seg_logger, "data_dir: %i, val: %i", data_dir, val);
		set_in_segment(segment, data_dir, size, &val);
	}
}

process_segment* find_segment_with_space(process_table* table , int size) {
	int is_heap(process_segment* segment) {
		return segment->type == HEAP;
	};
	int has_space(process_segment* segment) {
		int free_dir = find_free_dir(segment, size);
		return free_dir != -1;
	}

	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		uint32_t seg_pointer = segments + i * sizeof(process_segment);
		memcpy(segment, seg_pointer, sizeof(process_segment));
		if (is_heap(segment) && has_space(segment)) {
			free(segment);
			return seg_pointer;
		}
		i++;
	}
	free(segment);
	return NULL;
}

process_segment* segment_by_dir(process_table* table, int dir) {
	process_segment* segment = malloc(sizeof(process_segment));
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		uint32_t seg_pointer = segments + i * sizeof(process_segment);
		memcpy(segment, seg_pointer, sizeof(process_segment));
		if (segment->base < dir && (segment->base + segment->size) > dir) {
			free(segment);
			return seg_pointer;
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
	if (segment->type == HEAP) {
		free(segment);
		return segments + offset;
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

void* find_free_dir(process_segment* segment, int size) {
	log_info(seg_logger, "find_free_dir, size: %i", size);
	uint32_t dir = 0;
	uint32_t next_dir = 0;
	while (dir < segment->size) {
		bool is_free;
		int block_size;
		next_dir = get_metadata_from_segment(segment, dir, &is_free, &block_size);

		log_info(seg_logger, "dir: %i, is_free: %i, size: %i", dir, is_free, block_size);

		if (is_free) {
			if (block_size >= (size + metadata_size)) {
				log_error(seg_logger, "block_size: %i", block_size);
				log_error(seg_logger, "size + metadata_size: %i", size + metadata_size);
				return dir;
			} else {
				dir = next_dir + block_size;
			}
		} else {
			dir = next_dir + block_size;
		}
	}
	return -1;
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
	int offset = process_table->number_of_segments * sizeof(process_segment);
	if (segments == NULL) {
		process_table->segments = malloc(sizeof(process_segment));
	}
	realloc(process_table->segments, segments + offset + sizeof(process_segment));
	memcpy(process_table->segments + offset, segment, sizeof(process_segment));
	process_table->number_of_segments += 1;
	log_debug(seg_logger, "Se agrego un segmento con base: %i y size: %i al process: %s", segment->base, segment->size, process);
}

process_table* get_table_for_process(char* process) {
	int is_this(process_table* segment_table) {
		return string_equals_ignore_case(segment_table->process, process);
	}
	return list_find(tables, (void*) is_this);
}

int find_free_frame(t_bitarray* bitmap) {
	int var;
	for (var = 0; var < (MEMORY_SIZE / PAGE_SIZE); ++var) {
		bool is_used = bitarray_test_bit(bitmap, var);
//		log_info(seg_logger, "i = %i, is_used?: %i", var, is_used);
		if (!is_used) {
			return var;
		}
	}
	return -1;
}

int min(int x, int y) {
	return x < y ? x : y;
}

int max(int x, int y) {
	return x > y ? x : y;
}

int free_space_at_the_end(process_segment* segment) {
	uint32_t dir = 0;
	uint32_t next_dir = 0;
	while (dir < segment->size) {
		bool is_free;
		int block_size;
		next_dir = get_metadata_from_segment(segment, dir, &is_free, &block_size);
		if (is_free) {
			if (next_dir + block_size == segment->size) {
				log_error(seg_logger, "last_free_size_is: %i", block_size);
				return block_size;
			} else {
				dir = next_dir + block_size;
			}
		} else {
			dir = next_dir + block_size;
		}
	}
	return -1;
}

void* clear_in_segment(process_segment* segment, uint32_t dir, uint32_t size) {
	void* pages = segment->pages;
	int offset = dir % PAGE_SIZE;
	int page_number = (dir - offset) / PAGE_SIZE;

	t_page* page = malloc(sizeof(t_page));
	void* frame;

	int size_allocd = 0;
	while (size_allocd < size) {
		memcpy(page, segment->pages + page_number * sizeof(t_page), sizeof(t_page));
		frame = MEMORY[page->frame_number];

		int to_alloc = min(size - size_allocd, PAGE_SIZE - offset % PAGE_SIZE);
		memset(frame + offset, NULL, to_alloc);
		size_allocd += to_alloc;
		offset += to_alloc;
		page_number += offset / PAGE_SIZE;
		offset = offset % PAGE_SIZE;
	}

	return segment->base + page_number * PAGE_SIZE + offset;
}


void* set_in_segment(process_segment* segment, uint32_t dir, uint32_t size, void* value) {
	void* pages = segment->pages;
	int offset = dir % PAGE_SIZE;
	int page_number = (dir - offset) / PAGE_SIZE;

	t_page* page = malloc(sizeof(t_page));
	void* frame;

	int size_allocd = 0;
	while (size_allocd < size) {
		memcpy(page, segment->pages + page_number * sizeof(t_page), sizeof(t_page));
		frame = MEMORY[page->frame_number];
		int to_alloc = min(size - size_allocd, PAGE_SIZE - offset % PAGE_SIZE);
		memcpy(frame + offset, value + size_allocd, to_alloc);
		size_allocd += to_alloc;
		offset += to_alloc;
		page_number += offset / PAGE_SIZE;
		offset = offset % PAGE_SIZE;
	}

	return segment->base + page_number * PAGE_SIZE + offset;
}

void* get_from_segment(process_segment* segment, uint32_t dir, uint32_t size, void* to) {
	void* pages = segment->pages;
	int offset = dir % PAGE_SIZE;
	int page_number = (dir - offset) / PAGE_SIZE;

	t_page* page = malloc(sizeof(t_page));
	void* frame;

	int copied = 0;
	while (copied < size) {
		memcpy(page, segment->pages + page_number * sizeof(t_page), sizeof(t_page));
		frame = MEMORY[page->frame_number];
		int to_copy = min(size - copied, PAGE_SIZE - offset % PAGE_SIZE);
		memcpy(to + copied, frame + offset, to_copy);
		copied += to_copy;
		offset += to_copy;
		page_number += offset / PAGE_SIZE;
		offset = offset % PAGE_SIZE;
	}
	return segment->base + page_number * PAGE_SIZE + offset;
}

void* get_metadata_from_segment(process_segment* segment, uint32_t dir, bool* is_free, uint32_t* size) {
	uint32_t next_dir = dir;
	next_dir = get_from_segment(segment, next_dir, sizeof(bool), is_free);
	next_dir = get_from_segment(segment, next_dir, sizeof(uint32_t), size);
	return next_dir;
}

void* set_metadata_in_segment(process_segment* segment, uint32_t dir, bool is_free, uint32_t size) {
	uint32_t next_dir = dir;
	next_dir = set_in_segment(segment, next_dir, sizeof(bool), &is_free);
	next_dir = set_in_segment(segment, next_dir, sizeof(uint32_t), &size);
	return next_dir;
}



uint32_t alloc_in_segment(process_segment* segment, uint32_t process_dir, uint32_t size) {
	uint32_t dir = process_dir - segment->base;
//	log_debug(seg_logger, "alloc_in_segment, size: %i, dir: %i", size, dir);
	uint32_t metadata_end_dir;
	uint32_t allocated_end_dir;
	uint32_t free_metadata_end_dir;

	metadata_end_dir = set_metadata_in_segment(segment, dir, false, size);
	log_debug(seg_logger, "metadata_end_dir: %i", metadata_end_dir);

	allocated_end_dir = clear_in_segment(segment, metadata_end_dir, size);
	log_debug(seg_logger, "allocated_end_dir: %i", allocated_end_dir);

	int last_space_used = (allocated_end_dir + metadata_size) % PAGE_SIZE;
	int free_space_after = PAGE_SIZE - last_space_used;

	if (last_space_used == 0) {
		free_space_after = 0;
	}

	log_debug(seg_logger, "free_space_after: %i", free_space_after);

	free_metadata_end_dir = set_metadata_in_segment(segment, allocated_end_dir, true, free_space_after);
	log_debug(seg_logger, "free_metadata_end_dir: %i", free_metadata_end_dir);

//	bool is_f;
//	int s;
//	get_metadata_from_segment(segment, dir, &is_f, &s);
//	log_debug(seg_logger, "used, is_f: %i, s: %i", is_f, s);
//	get_metadata_from_segment(segment, allocated_end_dir, &is_f, &s);
//	log_debug(seg_logger, "free, is_f: %i, s: %i", is_f, s);

	return process_dir + metadata_end_dir;
}
