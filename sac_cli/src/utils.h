/*
 * utils.h
 *
 *  Created on: 10 nov. 2019
 *      Author: utnso
 */

#ifndef UTILS_H_
#define UTILS_H_

#include <time.h>
#include <stdint.h>

void convert_to_timespec(uint64_t microsecs, struct timespec* ts);

#endif /* UTILS_H_ */
