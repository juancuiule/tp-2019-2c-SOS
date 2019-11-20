#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, PORT);

	if (init_result != -1) {
		uint32_t my_memory = muse_alloc(10);
		int x = 10;
		int*y = malloc(4);
		muse_cpy(my_memory, &x, 4);
		muse_get(y, my_memory, 4);
		printf("y: %d \n", *y);
		muse_free(my_memory);
		

		muse_map("/Downloads", 100, MAP_PRIVATE);
		muse_sync(1000, 20);
		muse_unmap(5555);

		muse_close();

		return 0;
	} else {
		return EXIT_FAILURE;
	}
}