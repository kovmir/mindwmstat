/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifndef STATS_H
#define STATS_H
/* Query various system stats like RAM, power state, etc. */

#include <stdio.h>
#include <time.h>

/* All functions return -1 on failure, and 0 on success. */

/* Return system load in /proc/loadavg fashion. */
int load_query(char *out_buf, size_t out_len);
/* Get current system date & time. */
int time_query(char *out_buf, size_t out_len, const char *format, time_t t);
/* Open (file descriptors) battery, must be called prior to query_battery(). */
/* Get occupied RAM: 0 to 100%. */
int memory_query(int *out_buf, FILE *meminfo);

#endif /* STATS_H */
