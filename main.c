/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#include <assert.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifndef CONSOLE_OUTPUT
#include <X11/Xlib.h>
#endif /* CONSOLE_OUTPUT */

#include "stats.h"

#define SUPPORT_URL "https://github.com/kovmir/mindwmstat/issues"
#ifndef GIT_VERSION
#define GIT_VERSION "dev"
#endif /* GIT_VERSION */
#define LOAD_LEN 16
#define BATTERY_EXISTS() (cap_fd != -1 && ac_fd != -1)

#include "config.h"

static const char ac_icons[] = {
	[POWER_BATTERY] = charge_icon,
	[POWER_AC]      = discharge_icon,
};

#ifndef CONSOLE_OUTPUT
void
set_x11status(Display *display, char *status)
{
	assert(display != NULL);
	assert(status != NULL);
	XStoreName(display, XDefaultRootWindow(display), status);
	XSync(display, 0);
}
#endif /* CONSOLE_OUTPUT */

int
main(int argc, char *argv[])
{
	char load_buf[LOAD_LEN];
	char time_buf[TIME_LEN];
	char status_buf[STATUS_LEN];
	BatteryInfo bi;
	int mem_usage;
	size_t off;
#ifdef STATUS_ANIMATION
	int curr_frame = 0;
#endif /* STATUS_ANIMATION */

	int cap_fd = -1, ac_fd = -1;
	FILE *meminfo;
#ifndef CONSOLE_OUTPUT
	Display *display;
#endif /* CONSOLE_OUTPUT */

	(void)argv;
	if (argc > 1) {
		printf("%s\n", GIT_VERSION);
		printf("Support: %s\n", SUPPORT_URL);
		return 0;
	}

#ifndef CONSOLE_OUTPUT
	display = XOpenDisplay(NULL);
	if (display == NULL)
		errx(1, "unable to open X display");
#endif /* CONSOLE_OUTPUT */

	meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL)
		err(1, "unable to open /proc/meminfo");

	cap_fd = open(batt_path, O_RDONLY);
	if (cap_fd == -1)
		warn("battery not found %s", batt_path);
	if (cap_fd != -1) {
		/* Check AC power cord iff the battery has been found. */
		ac_fd = open(ac_path, O_RDONLY);
		if (cap_fd == -1)
			warn("AC plug not found %s", ac_path);
	}

	assert(status_delay > 0);
	for (;; sleep(status_delay))
	{
		if (get_load(load_buf, LOAD_LEN) == -1)
			errx(1, "unable to get CPU load");
		if (get_time(time_buf, TIME_LEN, time_format, time(NULL)) == -1)
			errx(1, "unable to get date and time");
		if (get_memory(&mem_usage, meminfo) == -1)
			errx(1, "unable to get RAM usage");
		if (BATTERY_EXISTS()) {
			if (get_battery(&bi, cap_fd, ac_fd) == -1)
				errx(1, "unable to get battery info");
		}

		snprintf(status_buf, STATUS_LEN, "%d%s%s%s%s",
		         mem_usage, status_delim,
		         load_buf,  status_delim,
		         time_buf);

		if (BATTERY_EXISTS()) {
			off = strlen(status_buf);
			snprintf(status_buf + off, STATUS_LEN - off, "%s%c%d",
			         status_delim,
			         ac_icons[bi.power_state], bi.capacity);
		}

#ifdef STATUS_ANIMATION
		off = strlen(status_buf);
		snprintf(status_buf + off, STATUS_LEN - off, " %c",
		         status_frames[curr_frame++]);
		curr_frame = curr_frame % N_FRAMES;
#endif /* STATUS_ANIMATION */

#ifdef CONSOLE_OUTPUT
		puts(status_buf);
#else
		set_x11status(display, status_buf);
#endif /* CONSOLE_OUTPUT */

		rewind(meminfo);
		lseek(cap_fd, 0, SEEK_SET);
		lseek(ac_fd, 0, SEEK_SET);
	}
}
