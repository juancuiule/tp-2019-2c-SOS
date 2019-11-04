#include <stdio.h>
#include <commons/collections/queue.h>
#include <pthread.h>

#include "servidor.h"
#include "utils.h"

typedef struct {
	void* data;
	size_t size;
} t_buffer;
