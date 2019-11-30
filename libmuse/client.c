#include "client.h"
#include "unistd.h"

int main(void) {
	config = config_create("./program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, PORT);

	if (init_result != -1) {
		uint32_t my_memory = muse_alloc(10);
//		uint32_t map_dir = muse_map("/Downloads", 24, MAP_PRIVATE);
//		uint32_t another_memory = muse_alloc(35);

		log_info(logger, "my_memory: %u", my_memory);
//		log_info(logger, "map_dir: %u", map_dir);
//		log_info(logger, "another_memory: %u", another_memory);
//		int x = 1998;
//		int*y = malloc(4);

//		muse_cpy(my_memory, &x, 4);
//		muse_get(y, my_memory, 4);
//		printf("y: %d \n", *y);
//		muse_free(my_memory);

//		uint32_t my_memory = muse_alloc(65);
//		uint32_t another_memory = muse_alloc(35);
////		uint32_t map_dir = muse_map("/Downloads", 80, MAP_PRIVATE);
////
//		log_info(logger, "my_memory: %u", my_memory);
////		log_info(logger, "another_memory: %u", another_memory);
//		int x = 1998;
//		int* y = malloc(4);
////
//		muse_cpy(another_memory, &x, 4);
//////		muse_get(y, my_memory, 4);
////		printf("y: %d \n", *y);
//		muse_free(my_memory);
//		muse_close();

		muse_close();

		return 0;
	} else {
		return EXIT_FAILURE;
	}
}
