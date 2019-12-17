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
#include <string.h>

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

//char* pasa_palabra(int cod) {
//	switch(cod) {
//		case 1:
//			return strdup("No sabes que sufro?\n");
//		case 2:
//			return strdup("No escuchas mi plato girar?\n");
//		case 3:
//			return strdup("Cuanto tiempo hasta hallarte?\n");
//		case 4:
//		case 5:
//			return strdup("Uh, haces mi motor andar\n");
//		case 6:
//			return strdup("Y mis cilindros rotar\n");
//		default: {
//			if(cod % 2)
//				return strdup("Oh si\n");
//			else
//				return strdup("un Archivo de swap supermasivo\n");
//		}
//	}
//}
//
//void recursiva(int num) {
//	if(num == 0) {
//		return;
//	}
//
//	char* estrofa = pasa_palabra(num);
//	int longitud = strlen(estrofa)+1;
//
//	uint32_t ptr = muse_alloc(longitud);
//
//	muse_cpy(ptr, estrofa, longitud);
//	recursiva(num - 1);
//	muse_get(estrofa, ptr, longitud);
//
//	puts(estrofa);
//	muse_free(ptr);
//	free(estrofa);
//	sleep(1);
//}
//
//int main(void) {
//	config = config_create("../src/program.config");
//
//	char* IP = config_get_string_value(config, "IP");
//	char* PORT = config_get_string_value(config, "PORT");
//
//	int init_result = muse_init((int) getpid(), IP, PORT);
//
//	if (init_result != -1) {
//		recursiva(15);
//		muse_close();
//		return 0;
//	} else {
//		return EXIT_FAILURE;
//	}
//}
