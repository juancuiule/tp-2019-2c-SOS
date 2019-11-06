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
#include <commons/log.h>
#include <commons/collections/dictionary.h>

t_config* config;
char* LISTEN_IP;
char* LISTEN_PORT;
int METRICS_TIMER;
int MAX_MULTIPROG;
char** SEM_IDS;
int* SEM_INIT;
int* SEM_MAX;
double ALPHA_SJF;

t_config* config_metricas_sistema;
char* GRADO_MULTIPROGRAMACION;

void configurar();

#endif /* CONFIGURACION_H_ */
