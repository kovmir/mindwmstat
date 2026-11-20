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
	int        capacity; /* Battery capacity 0 to 100%. */
	PowerState power_state;
} BatteryInfo;

/* Battery handle. */
typedef struct {
	int cap_fd;
	int ac_fd;
} Battery;

/* All functions return -1 on failure, and 0 on success. */

/* Return system load in /proc/loadavg fashion. */
int load_query(char *out_buf, size_t out_len);
/* Get current system date & time. */
int time_query(char *out_buf, size_t out_len, const char *format, time_t t);
/* Open (file descriptors) battery, must be called prior to query_battery(). */
int battery_open(const char *batt_path, const char *ac_path,
                 Battery *batt, const char **failed_path);
/* Query power state. */
int battery_query(Battery batt, BatteryInfo *bi);
/* Close (release file descriptors) battery when no longer needed. */
void battery_close(Battery *batt);
/* Get occupied RAM: 0 to 100%. */
int memory_query(int *out_buf, FILE *meminfo);

#endif /* STATS_H */
