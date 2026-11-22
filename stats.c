/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#include <assert.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "stats.h"

int
load_query(char *out_buf, size_t out_len)
{
	double load[3];

	assert(out_buf != NULL);

	if (getloadavg(load, 3) != 3)
		return -1;
	snprintf(out_buf, out_len,
	         "%.2f %.2f %.2f", load[0], load[1], load[2]);
	return 0;
}

int
time_query(char *out_buf, size_t out_len, const char *format, time_t t)
{
	struct tm *tm;

	assert(out_buf != NULL);
	assert(format != NULL);

	tm = localtime(&t);
	if (tm == NULL)
		return -1;
	if (strftime(out_buf, out_len, format, tm) == 0)
		return -1;
	return 0;
}

int
memory_query(int *out_buf, FILE *meminfo)
{
	char buf[1024];
	long mem_total = -1, mem_avail = -1;

	assert(out_buf != NULL);
	assert(meminfo != NULL);

	while (fgets(buf, sizeof(buf), meminfo)) {
		if (sscanf(buf, "MemTotal: %ld kB", &mem_total) == 1)
			continue;
		if (sscanf(buf, "MemAvailable: %ld kB", &mem_avail) == 1)
			continue;
		if (mem_total != -1 && mem_avail != -1)
			break; /* Found everything. */
	}
	if (mem_total == -1 || mem_avail == -1) {
		return -1;
	}
	if (mem_total == 0) { /* Pointless? */
		return -1;
	}

	*out_buf = (int)(100L * (mem_total - mem_avail) / mem_total);
	return 0;
}
