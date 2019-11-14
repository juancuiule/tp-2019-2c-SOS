#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	muse_init((int) getpid(), IP, PORT);

	void* mem_muse = muse_alloc(5);
	
	log_info(logger, "dir de mem_muse: %u", mem_muse);
	void* x = malloc(5);

	muse_get(x, mem_muse, 5);
	log_info(logger, "dir de mem_muse: %u", mem_muse);
	log_info(logger, "x es: %s", x);

	muse_free(mem_muse);

	muse_close();

	return 0;
}