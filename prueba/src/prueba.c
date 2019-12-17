/*
 ============================================================================
 Name        : prueba.c
 Author      :
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <libmuse/libmuse.h>
#include <commons/config.h>
#include "unistd.h"

t_config* config;

void recursiva(int num) {
	if (num == 0) {
		return;
	}

	uint32_t ptr = muse_alloc(4);
	muse_cpy(ptr, &num, 4);
	printf("%d\n", num);

	sleep(1);
	recursiva(num - 1);

	num = 0;
	muse_get(&num, ptr, 4);
	printf("%d\n", num);
	muse_free(ptr);
}

int main(void) {
	config = config_create("../src/program.config");

	char* IP = config_get_string_value(config, "IP");
	char* PORT = config_get_string_value(config, "PORT");

	int init_result = muse_init((int) getpid(), IP, PORT);

	if (init_result != -1) {
		recursiva(10);
		muse_close();
		return 0;
	} else {
		return EXIT_FAILURE;
	}
}

