#include "segmentpag.h"
#include <math.h>

metadata_size = sizeof(bool) + sizeof(uint32_t);

bool t = true;
bool f = false;

void init_structures(int m_size, int p_size) {
	init_log();
	init_tables();
	init_mutexs();
	init_memoria();
	init_virtual();
}

void init_log() {
	seg_logger = log_create("../logs/segmentpag.log", "segmentpag", 1, LOG_LEVEL_DEBUG);
}

void init_tables() {
	tables = list_create();
}

void init_memoria() {
	// Memoria
	int frames = MEMORY_SIZE / PAGE_SIZE;
	int bitmap_size_in_bytes = ceil((double) frames / 8);
	bitmap_pointer = malloc(bitmap_size_in_bytes);
	frame_usage_bitmap = bitarray_create_with_mode(bitmap_pointer, bitmap_size_in_bytes, LSB_FIRST);

	clear_bitmap(frame_usage_bitmap, frames);

	MEMORY = calloc(frames, PAGE_SIZE);
	int i;
	for(i = 0; i < frames; i++) {
		void* new_frame = malloc(PAGE_SIZE);
		memset(new_frame, 1, sizeof(bool));
		memset(new_frame + sizeof(bool), NULL, PAGE_SIZE - sizeof(bool));
		*(MEMORY + i) = new_frame;
	}
}

void init_virtual() {
	// Memoria Virtual
	swap_file = fopen("file.bin", "rw+b");

	int swap_frames = SWAP_SIZE / PAGE_SIZE;
	int swap_bitmap_size_in_bytes = ceil((double) swap_frames / 8);
	swap_bitmap_pointer = malloc(swap_bitmap_size_in_bytes);
	swap_usage_bitmap = bitarray_create_with_mode(swap_bitmap_pointer, swap_bitmap_size_in_bytes, LSB_FIRST);

	clear_bitmap(swap_usage_bitmap, swap_frames);
}

void init_mutexs() {
	pthread_mutex_init(&memory_frames_bitarray, NULL);
	pthread_mutex_init(&swap_frames_bitarray, NULL);
	pthread_mutex_init(&mutex_paginas_en_memoria, NULL);
	pthread_mutex_init(&mutex_asignar_pagina, NULL);
}

void clear_bitmap(t_bitarray* bitmap, int bits) {
	for (int var = 0; var < bits; var++) {
		bitarray_clean_bit(bitmap, var);
	}
}


// Create
t_page *create_page() {
	t_page * page = malloc(sizeof(t_page));
	page->flag = true;
	page->frame_number = -1;
	page->in_use = 1;
	page->modified = 0;
	return page;
}

process_segment *create_segment(segment_type type, uint32_t base) {
	process_segment *segment = malloc(sizeof(process_segment));
	segment->type = type;
	segment->base = base;
	segment->size = 0;
	segment->pages= NULL;
	log_info(seg_logger, "Nuevo segmento (%s) creado. base: %i, size: %i", type == HEAP ? "HEAP" : "MMAP", base, 0);
	return segment;
}

void create_process_table(char* process) {
	process_table* new_table = malloc(sizeof(process_table));
	new_table->process = process;
	new_table->segments = NULL;
	new_table->number_of_segments = 0;
	list_add(tables, new_table);
	log_info(seg_logger, "Se creo una process_table para el proceso: %s", process);
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

	log_info(seg_logger, "Se agrego una página al segmento con base: %i, nuevo size: %i", segment->base, segment->size);

	free(page);
}

void free_dir(process_segment* segment, uint32_t dir) {
	int page_dir = dir - segment->base;
	int metadata_dir = page_dir - metadata_size;

	bool is_free;
	uint32_t data_size;

	get_metadata_from_segment(segment, metadata_dir, &is_free, &data_size);

	if (is_free) {
		log_error(seg_logger, "Ya estaba sin asignar");
	} else {
		int offset = metadata_dir + metadata_size + data_size;
		bool next_is_free = true;
		uint32_t next_data_size;
		uint32_t new_data_size = data_size;
		while (offset < segment->size && next_is_free) {
			get_metadata_from_segment(segment, offset, &next_is_free, &next_data_size);
			offset += next_data_size + metadata_size;
			if (next_is_free) {
				new_data_size += next_data_size + metadata_size;
			}
		}
		set_metadata_in_segment(segment, metadata_dir, true, new_data_size);
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
	}
	return *data;
}

void cpy_to_dir(process_segment* segment, uint32_t dir, void* val, int size) {
	// "retrocedo" para poder ver si esta libre y cuanto espacio tiene ese bloque
	int metadata_dir = dir - metadata_size;

	// Hay que chequear si la dir es valida.
	bool is_free;
	uint32_t data_size;

	uint32_t data_dir = get_metadata_from_segment(segment, metadata_dir, &is_free, &data_size);

	if (is_free) {
		log_error(seg_logger, "No hay un malloc hecho...");
	} else if (data_size < size) {
		log_error(seg_logger, "No hay espacio suficiente...");
	} else {
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

	process_segment* segment;
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		segment = segments + i * sizeof(process_segment);
		if (is_heap(segment) && has_space(segment)) {
			return segment;
		}
		i++;
	}
	return NULL;
}

process_segment* segment_by_dir(process_table* table, int dir) {
	process_segment* segment;
	void* segments = table->segments;
	int i = 0;
	while (i < table->number_of_segments) {
		segment = segments + i * sizeof(process_segment);
		if (segment->base < dir && (segment->base + segment->size) > dir) {
			return segment;
		}
		i++;
	}
	return NULL;
}

process_segment* find_extensible_heap_segment(process_table* table) {
	void* segments = table->segments;
	int offset = (table->number_of_segments - 1) * sizeof(process_segment);
	process_segment* segment = segments + offset;
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
	process_segment* segment;
	while (i < process_table->number_of_segments) {
		segment = segments + i * sizeof(process_segment);
		last += segment->size;
		i++;
	}
	return last;
}

void* find_free_dir(process_segment* segment, int size) {
	uint32_t dir = 0;
	uint32_t next_dir = 0;
	while (dir < segment->size) {
		bool is_free;
		int block_size;
		next_dir = get_metadata_from_segment(segment, dir, &is_free, &block_size);

		if (is_free) {
			if (block_size >= (size + metadata_size)) {
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
	log_info(seg_logger, "Se agrego un segmento con base: %i y size: %i al process: %s", segment->base, segment->size, process);
	free(segment);
}

process_table* get_table_for_process(char* process) {
	int is_this(process_table* segment_table) {
		return string_equals_ignore_case(segment_table->process, process);
	}
	return list_find(tables, (void*) is_this);
}

int find_free_bit(t_bitarray* bitmap, int limit) {
	for (int var = 0; var < limit; ++var) {
		bool is_used = bitarray_test_bit(bitmap, var);
		if (!is_used) {
			return var;
		}
	}
	return -1;
}

int find_free_frame() {
	pthread_mutex_lock(&memory_frames_bitarray);
	int free_bit = find_free_bit(frame_usage_bitmap, MEMORY_SIZE / PAGE_SIZE);
	pthread_mutex_unlock(&memory_frames_bitarray);
	return free_bit;
}

int find_free_swap() {
	pthread_mutex_lock(&swap_frames_bitarray);
	int free_bit = find_free_bit(swap_usage_bitmap, SWAP_SIZE / PAGE_SIZE);
	pthread_mutex_unlock(&swap_frames_bitarray);
	return free_bit;
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

	t_page* page;
	void* frame;

	int size_allocd = 0;
	while (size_allocd < size) {
		page = segment->pages + page_number * sizeof(t_page);
		page->in_use = 0;
		page->modified = 1;
		if (!page->flag) {
			asignar_frame(page);
		}
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

	t_page* page;
	void* frame;

	int size_allocd = 0;
	while (size_allocd < size) {
		page = segment->pages + page_number * sizeof(t_page);
		if (!page->flag) {
			asignar_frame(page);
		}
		frame = MEMORY[page->frame_number];

		int to_alloc = min(size - size_allocd, PAGE_SIZE - offset % PAGE_SIZE);
		memcpy(frame + offset, value + size_allocd, to_alloc);
		page->modified = 1;
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

	t_page* page;
	void* frame;

	int copied = 0;
	while (copied < size) {
		page = segment->pages + page_number * sizeof(t_page);
		page->in_use = 1;
		if (!page->flag) {
			asignar_frame(page);
		}
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
	uint32_t metadata_end_dir;
	uint32_t allocated_end_dir;
	uint32_t free_metadata_end_dir;

	metadata_end_dir = set_metadata_in_segment(segment, dir, false, size);

	allocated_end_dir = clear_in_segment(segment, metadata_end_dir, size);

	int last_space_used = (allocated_end_dir + metadata_size) % PAGE_SIZE;
	int free_space_after = PAGE_SIZE - last_space_used;

	if (last_space_used == 0) {
		free_space_after = 0;
	}

	free_metadata_end_dir = set_metadata_in_segment(segment, allocated_end_dir, true, free_space_after);

	return segment->base + metadata_end_dir;
}

t_list* paginas_en_memoria() {
	t_list* op(t_list* acum, process_table* process) {
		t_list* paginas = list_create();
		int i;
		process_segment* segment;
		t_page* pagina;
		for (i = 0; i < process->number_of_segments; ++i) {
			segment = process->segments + i * sizeof(process_segment);
			int j;
			for (j = 0; j < segment->size / PAGE_SIZE; j++) {
				pagina = segment->pages + j * sizeof(t_page);
				if (pagina->flag == true) {
					list_add(paginas, segment->pages + j * sizeof(t_page));
				}
			}
		}

		list_add_all(acum, paginas);
		return acum;
	}
	pthread_mutex_lock(&mutex_paginas_en_memoria);
	t_list* paginas = list_fold(tables, list_create(), (void*) op);
	pthread_mutex_unlock(&mutex_paginas_en_memoria);
	return paginas;
}

t_page* victima_0_0() {
	t_list* paginas = paginas_en_memoria();
	for(int i = 0; i < paginas->elements_count; i++) {
		t_page* pagina_en_memoria = list_get(paginas, i);
		if (
		  pagina_en_memoria->in_use == false &&
		  pagina_en_memoria->modified == false
		) {
			return pagina_en_memoria;
		}
	}
	return -1;
}

t_page* victima_0_1() {
	t_list* paginas = paginas_en_memoria();
	for(int i = 0; i < paginas->elements_count; i++) {
		t_page* pagina_en_memoria = list_get(paginas, i);
		if (
		  pagina_en_memoria->in_use == false &&
		  pagina_en_memoria->modified == true
		) {
			return pagina_en_memoria;
		} else {
			pagina_en_memoria->in_use = false;
		}
	}
	return -1;
}

void asignar_frame(t_page* pagina) {
	pthread_mutex_lock(&mutex_asignar_pagina);
	//  log_debug(seg_logger, "pagina frame: %i", pagina->frame_number);
	int frame_number = find_free_frame();

	if (frame_number == -1) {
		// no hay frame libre.. hay que hacer swap
		// Busco (0, 0)
		t_page* victima = victima_0_0();

		if (victima == -1) {
			victima = victima_0_1();
			if (victima == -1) {
				victima = victima_0_0();
			}
		}

		int frame_number_victima_pre_swap = victima->frame_number;

		// pasarla a swap_file
		int free_swap = find_free_swap();

		if (free_swap == -1) {
			// no hay mas espacio en swap
			log_error(seg_logger, "No hay mas espacio en swap");
		} else {
			log_debug(seg_logger, "free swap frame: %i, offset: %i", free_swap, free_swap * PAGE_SIZE);
			fseek(swap_file, free_swap * PAGE_SIZE, SEEK_SET);
			fwrite(MEMORY[victima->frame_number], sizeof(PAGE_SIZE), 1, swap_file);

			victima->flag = false;
			victima->frame_number = free_swap;

			bitarray_set_bit(swap_usage_bitmap, free_swap);

			// copiar en este frame lo que la pagina tenía en swap_file
			if (pagina->frame_number != -1 && !pagina->flag) {
				log_debug(seg_logger, "la pagina que pide frames tenía data en swap en la pagina: %i", pagina->frame_number);
				fseek(swap_file, pagina->frame_number * PAGE_SIZE, SEEK_SET);
				fread(MEMORY[frame_number_victima_pre_swap], sizeof(PAGE_SIZE), 1, swap_file);
				bitarray_clean_bit(swap_usage_bitmap, pagina->frame_number);
			}
			pagina->frame_number = frame_number_victima_pre_swap;
			pagina->in_use = true;
			pagina->flag = true;
			pagina->modified = false;

			log_debug(seg_logger, "Asigno el frame: %i que tenía otra página, esa página quedo en el frame %i del swap", frame_number_victima_pre_swap, free_swap);
		}
	} else {
		// hay frame libre
		pagina->frame_number = frame_number;
		bitarray_set_bit(frame_usage_bitmap, frame_number);

		log_debug(seg_logger, "Asigno el frame: %i", frame_number);
	}
	pthread_mutex_unlock(&mutex_asignar_pagina);
}

void print_process(process_table* table) {
//	log_info(seg_logger, "Proceso:");
//	log_info(seg_logger, "cantidad de segmentos: %i", table->number_of_segments);
//	log_info(seg_logger, "process id: %s", table->process);
//	process_segment* seg = malloc(sizeof(process_segment));
//	for (int var = 0; var < table->number_of_segments; ++var) {
//		memcpy(seg, table->segments + var * sizeof(process_segment), sizeof(process_segment));
//		print_segment(seg);
//	}
//	free(seg);
}

void print_segment(process_segment* segment) {
//	log_info(seg_logger, "Segmento:");
//	log_info(seg_logger, "base: %i", segment->base);
//	log_info(seg_logger, "size: %i", segment->size);
//	log_info(seg_logger, "type: %s", segment->type == HEAP ? "HEAP" : "MMAP");
//
//	int read = 0;
//	while (read < segment->size) {
//		bool is_free;
//		uint32_t size;
//		read = get_metadata_from_segment(segment, read, &is_free, &size);
//		log_info(seg_logger, "dir: %i, is_free: %i, size: %i", read, is_free, size);
//		read += size;
//	}
}
