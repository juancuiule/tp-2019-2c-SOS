#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	muse_init((int) getpid(), IP, PORT);

	void* algo = muse_alloc(5);
	void* otro = malloc(5);
	log_info(logger, "algo: %u", algo);
	log_info(logger, "otro: %s", otro);
	muse_get(otro, algo, 5);
	log_info(logger, "otro despues de copy: %s", otro);

	muse_close();

	return 0;
}