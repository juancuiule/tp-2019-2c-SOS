#include "client.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	muse_init(1, IP, PORT);

	muse_alloc(10);

	muse_close();

	return 0;
}
