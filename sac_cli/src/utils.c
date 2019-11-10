#include "utils.h"

void convert_to_timespec(uint64_t microsecs, struct timespec* ts)
{
	int secs = microsecs / 1000000;
	int s = (int) secs;

	ts->tv_sec = s;
	ts->tv_nsec = (secs -s) * 1000000000;
}
