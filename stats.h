/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifndef STATS_H
#define STATS_H
/* Query various system stats like RAM, power state, etc. */

#include <stdio.h>
#include <time.h>

/* Power sources. */
typedef enum {
	POWER_BATTERY, /* Not charging. */
	POWER_AC,      /* Charging. */
} PowerState;

/* Battery state. */
typedef struct {
	int        capacity; /* Battery capacity: 0 to 100%. */
	PowerState power_state;
} BatteryInfo;

/* All functions return -1 on failure, and 0 on success. */

/* Return system load in /proc/loadavg fashion. */
int get_load(char *out_buf, size_t out_len);
/* Get current system date & time. */
int get_time(char *out_buf, size_t out_len, const char *format, time_t t);
/* Query power state. */
int get_battery(BatteryInfo *bi, int cap_fd, int ac_fd);
/* Get occupied RAM: 0 to 100%. */
int get_memory(int *out_buf, FILE *meminfo);

#endif /* STATS_H */
