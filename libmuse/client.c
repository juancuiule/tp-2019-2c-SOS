#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, PORT);

	if (init_result != -1) {
		void* mem_muse = muse_alloc(5);
		char* x = malloc(5);

		muse_cpy(mem_muse, "Hola", 5);

		muse_get(x, mem_muse, 5);
		log_info(logger, "x es: %s", x);

		muse_cpy(mem_muse, "Ciao", 5);

		muse_get(x, mem_muse, 5);
		log_info(logger, "x es: %s", x);

		muse_free(mem_muse);

		muse_close();

		return 0;
	} else {
		return EXIT_FAILURE;
	}
}