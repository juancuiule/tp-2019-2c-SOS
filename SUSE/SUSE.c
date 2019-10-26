#include "SUSE.h"

int main() {
	pthread_t scheduler_thread;
	configurar();
	inicializar_colas();
	pthread_create(&scheduler_thread, NULL, (void*)scheduler, NULL);
	pthread_join(&scheduler_thread, NULL);
	return 0;
}
