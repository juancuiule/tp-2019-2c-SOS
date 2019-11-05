/*
 * configuracion.h
 *
 *  Created on: 8 oct. 2019
 *      Author: utnso
 */

#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <stdio.h>
#include <commons/config.h>

t_config* config;
t_log* logger;
char* LISTEN_IP;
char* LISTEN_PORT;
int METRICS_TIMER;
int MAX_MULTIPROG;
char** SEM_IDS;
int* SEM_INIT;
int* SEM_MAX;
double ALPHA_SJF;

void configurar();
void liberar();

#endif /* CONFIGURACION_H_ */
