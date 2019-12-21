#include <stdio.h>
#include <stdlib.h>
#include "libmuse.h"
#include <hilolay/hilolay.h>
#include <string.h>

#define RUTA_ARCHIVO "./prueba"
#define MAP_PRIVATE 1

struct hilolay_sem_t* presion_emitida;
struct hilolay_sem_t* presion_recibida;
struct hilolay_sem_t* revolucion_emitida;
struct hilolay_sem_t* revolucion_recibida;

uint32_t leer_archivo(uint32_t arch, uint32_t leido)
{
	uint32_t len;
	char * palabra = malloc(100);

	muse_get(&len, arch + leido, sizeof(uint32_t));
	leido += sizeof(uint32_t);
	muse_get(palabra, arch + leido, len);
	leido += len;

	printf(palabra);
	free(palabra);
	return leido;
}

void *revolucionar()
{
	hilolay_wait(presion_emitida);
	uint32_t arch = muse_map(RUTA_ARCHIVO, 4096, MAP_PRIVATE);
	uint32_t offset = 0;
	uint32_t size;

	muse_get(&size, arch, sizeof(uint32_t));
	offset = sizeof(uint32_t);
	while(offset < size)
		offset = leer_archivo(arch, offset);

	arch += 5000;

	muse_get(NULL, arch, 1);

	muse_sync(arch, 4096);

	muse_unmap(arch);
	return 0;
}


int main(void)
{
	struct hilolay_t revolucion;

	hilolay_init();
	muse_init(2, "127.0.0.1", 3306);

	presion_emitida = hilolay_sem_open("presion_emitida");
	presion_recibida = hilolay_sem_open("presion_recibida");
	revolucion_emitida = hilolay_sem_open("revolucion_emitida");
	revolucion_recibida = hilolay_sem_open("revolucion_recibida");

	hilolay_create(&revolucion, NULL, &revolucionar, NULL);

	hilolay_join(&revolucion);

	hilolay_sem_close(presion_emitida);
	hilolay_sem_close(presion_recibida);
	hilolay_sem_close(revolucion_emitida);
	hilolay_sem_close(revolucion_recibida);

	return hilolay_return(0);
}
