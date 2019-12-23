#ifndef SERVER_H_
#define SERVER_H_


#include <stdlib.h>
#include <pthread.h>
#include <common/protocol.h>
#include <common/log.h>
#include "sac_op.h"

#define SAC_PORT 8048
#define COD_PROCESO 'S'

void servidor_iniciar(int port);

#endif /* SERVER_H_ */
