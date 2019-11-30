#include "utils.h"

void convert_to_timespec(uint64_t millsecs, struct timespec* ts)
{
	int secs = millsecs / 1000;
	int s = (int) secs;

	ts->tv_sec = s;
	ts->tv_nsec = (secs -s) * 1000000;
}
