#include <stdio.h>
#include <stdlib.h>
#include "libmuse.h"
#include <hilolay/hilolay.h>
#include <string.h>
#include <unistd.h>

#define RUTA_ARCHIVO "map_privado"
#define MAP_PRIVATE 1

struct hilolay_sem_t *presion_emitida;
struct hilolay_sem_t *presion_recibida;

void grabar_archivo(uint32_t arch, char* palabra)
{
	uint32_t offset;
	uint32_t tam = strlen(palabra) + 1;
	muse_get(&offset, arch, sizeof(uint32_t));
	muse_cpy(arch + offset, &tam, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	muse_cpy(arch + offset, palabra, tam);
	offset += tam;
	muse_cpy(arch, &offset, sizeof(uint32_t));
	sleep(1);
}

void *presionar()
{
	uint32_t arch = muse_map(RUTA_ARCHIVO, 4096, MAP_PRIVATE);
	uint32_t offset = sizeof(uint32_t);

	muse_cpy(arch, &offset, sizeof(uint32_t));

	grabar_archivo(arch, "Estas atrapado con la espalda contra la pared\n");

	grabar_archivo(arch, "No hay solucion ni salida\n");

	grabar_archivo(arch, "Estas luchando y nadie puede ver\n");

	grabar_archivo(arch, "La presion crece exponencialmente\n");

	grabar_archivo(arch, "Intentas mantener el paso y no podes\n");

	grabar_archivo(arch, "No aprobaras. Muajaja\n");

	muse_sync(arch, 4096);

	muse_unmap(arch);
	hilolay_signal(presion_emitida);
	return 0;
}


int main(void)
{
	struct hilolay_t presion;

	hilolay_init();
	muse_init(getpid(), "127.0.0.1", 3306);

	presion_emitida = hilolay_sem_open("presion_emitida");
	presion_recibida = hilolay_sem_open("presion_recibida");

	hilolay_create(&presion, NULL, &presionar, NULL);

	hilolay_join(&presion);

	hilolay_sem_close(presion_emitida);
	hilolay_sem_close(presion_recibida);

	return hilolay_return(0);
}
