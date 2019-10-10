#include "SUSE.h"

int main() {
	pthread_t server_thread;

	configurar();

	pthread_create(&server_thread, NULL, (void*)server, NULL);

	pthread_join(&server_thread, NULL);

	return 0;
}
