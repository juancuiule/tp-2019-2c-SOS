#include "SUSE.h"

int main() {
	pthread_t scheduler_thread;
	configurar();
	inicializar_colas();
	scheduler();
	return 0;
}
