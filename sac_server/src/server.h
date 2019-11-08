/*
 * server.h
 *
 *  Created on: 7 nov. 2019
 *      Author: utnso
 */

#ifndef SERVER_H_
#define SERVER_H_


#include <stdlib.h>
#include <pthread.h>
#include <common/protocol.h>
#include <common/log.h>
#include "sac_op.h"

#define SAC_PORT 8048
#define COD_PROCESO 'S'

void servidor_iniciar();

#endif /* SERVER_H_ */
