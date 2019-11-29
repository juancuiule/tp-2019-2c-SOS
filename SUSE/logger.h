/*
 * logger.h
 *
 *  Created on: 29 nov. 2019
 *      Author: utnso
 */

#ifndef LOGGER_H_
#define LOGGER_H_

#include <commons/log.h>
#include "configuracion.h"
#include "estructuras.h"

t_log* logger;
t_log* logger_metricas;

void inicializar_loggers();
void logear_metricas(int grado_multiprogramacion);

#endif /* LOGGER_H_ */
