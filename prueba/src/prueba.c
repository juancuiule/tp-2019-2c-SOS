#include "prueba.h"
#include "commons/config.h"

t_config* config;

void recursiva(int num) {
	if (num == 0) {
		return;
	}

	uint32_t ptr = muse_alloc(4);
	muse_cpy(ptr, &num, 4);
	printf("%d\n", num);

	recursiva(num - 1);

	num = 0;
	muse_get(&num, ptr, 4);
	printf("%d\n", num);
	muse_free(ptr);
}

int main(void) {
	printf("Hola");
	config = config_create("../src/program.config");
	printf("Hola");
	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, (void*) PORT);
//
//	if (init_result != -1) {
//		recursiva(10);
//		muse_close();
//		return 0;
//	} else {
//		return EXIT_FAILURE;
//	}
}
