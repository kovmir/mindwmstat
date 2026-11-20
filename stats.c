/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

#include "stats.h"

static int read_int(int *out_buf, int fd);

static int
read_int(int *out_buf, int fd)
{
	char *endp;
	size_t n_read;
	char buf[32];
	long int value;
	ssize_t r;

	assert(out_buf != NULL);
	assert(fd >= 0);

	/* Read the file. Loop to handle short reads and EINTR. */
	n_read = 0;
	while (n_read < sizeof(buf)-1) {
		r = read(fd, buf + n_read, sizeof(buf)-1 - n_read);
		if (r == -1) {
			if (errno == EINTR)
				continue; /* Interrupted: retry. */
			return -1;
		}
		if (r == 0)
			break; /* EOF. */
		n_read += (size_t)r;
	}
	if (n_read == 0)
		return -1;
	buf[n_read] = '\0';

	/* Parse integer from the line. */
	errno = 0;
	value = strtol(buf, &endp, 10);

	/* Error checking... */
	if (buf == endp)
		return -1; /* No integer found. */
	if (errno != 0)
		return -1; /* Parse error / strtol overflow. */
	if (value > INT_MAX || value < INT_MIN)
		return -1; /* Integer overflow — fits in long but not int. */
	/* Reject trailing garbage. "123abc" should not silently parse as 123. */
	while (*endp != '\0') {
		if (*endp != ' ' && *endp != '\t' &&
		    *endp != '\n' && *endp != '\r')
			return -1; /* Trailing garbage. */
		endp++;
	}

	/* Good. */
	*out_buf = value;
	return 0;
}

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
battery_open(const char *batt_path, const char *ac_path,
             Battery *batt, const char **failed_path)
{
	int tmp;

	assert(batt_path != NULL);
	assert(ac_path != NULL);
	assert(batt != NULL);

	batt->cap_fd = -1;
	batt->ac_fd = -1;
	tmp = open(batt_path, O_RDONLY);
	if (tmp == -1) {
		if (failed_path != NULL)
			*failed_path = batt_path;
		return -1;
	}
	batt->cap_fd = tmp;
	tmp = open(ac_path, O_RDONLY);
	if (tmp == -1) {
		if (failed_path != NULL)
			*failed_path = ac_path;
		close(batt->cap_fd);
		batt->cap_fd = -1;
		return -1;
	}
	batt->ac_fd = tmp;
	return 0;
}

int
battery_query(Battery batt, BatteryInfo *bi)
{
	int tmp;

	assert(batt.cap_fd >= 0);
	assert(batt.ac_fd >= 0);

	if (lseek(batt.cap_fd, 0, SEEK_SET) ||
	    lseek(batt.ac_fd, 0, SEEK_SET))
		return -1;
	if (read_int(&tmp, batt.cap_fd) != 0)
		return -1;
	bi->capacity = tmp;
	if (read_int(&tmp, batt.ac_fd) != 0)
		return -1;
	bi->power_state = (tmp == 1) ? POWER_AC : POWER_BATTERY;
	return 0;
}

void
battery_close(Battery *batt)
{
	assert(batt != NULL);
	if (batt->cap_fd != -1) {
		close(batt->cap_fd);
		batt->cap_fd = -1;
	}
	if (batt->ac_fd != -1) {
		close(batt->ac_fd);
		batt->ac_fd = -1;
	}
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
