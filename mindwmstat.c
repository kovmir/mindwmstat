/* Includes */
#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef CONSOLE_OUTPUT
#include <X11/Xlib.h>
#endif /* CONSOLE_OUTPUT */

/* Constants and macros */
#define LOAD_LEN 15
#define STATUS_LEN 95
#define TIME_LEN 30
#define AC_STATE_CHARGING '1'

/* Function prototypes */
/* Get average CPU load; return true on success. */
static bool get_load(char *buf);
/* Get local time; return true on success. */
static bool get_time(char *buf);
/* Get current battery capacity percentage; return true on success. */
static bool get_charge(int *batt_charge);
/* Get AC power plug state; return true on success. */
static bool is_charging(bool *ac_state);
/* Get RAM usage percentage; return true on success. */
static bool get_ram(int *ram_usage);
/* Set dwm status; return true on success. */
static bool set_status(char *status);

#include "config.h"

bool
get_load(char *buf)
{
	double load[3];

	if (getloadavg(load, 3) != 3) {
		warnx("unable to get cpu load");
		return false;
	}
	snprintf(buf, LOAD_LEN, "%.2f %.2f %.2f", load[0], load[1], load[2]);
	return true;
}

bool
get_time(char *buf)
{
	time_t t = time(NULL);

	struct tm *tm = localtime(&t);
	if (!tm) {
		warnx("unable to get time (localtime)");
		return false;
	}
	if (!strftime(buf, TIME_LEN, time_format, tm)) {
		warnx("unable to get time (strftime)");
		return false;
	}
	return true;
}

bool
get_charge(int *batt_charge)
{
	FILE     *batt;
	const int BUF_SIZE = 16;
	char      buf[BUF_SIZE];

	if (batt_charge == NULL) {
		warnx("batt_charge cannot be NULL");
		return false;
	}

	batt = fopen(batt_path, "r");
	if (batt == NULL) {
		/* Pollutes stderr on devices without battery. */
		/* warn("unable to open %s", batt_path); */
		return false;
	}
	if (fgets(buf, BUF_SIZE, batt) == NULL) {
		warnx("unable to read %s", batt_path);
		fclose(batt);
		return false;
	}
	fclose(batt);

	/* Error checking is pointless, I think. */
	*batt_charge = (int)strtol(buf, NULL, 10);
	return true;
}

bool
is_charging(bool *charging)
{
	FILE *ac;
	char  state;

	if (charging == NULL) {
		warnx("ac_state cannot be NULL");
		return false;
	}

	ac = fopen(ac_path, "r");
	if (ac == NULL) {
		warn("unable to open %s", ac_path);
		return false;
	}
	if ((state = fgetc(ac)) == EOF) {
		warnx("unable to read AC power plug state");
		fclose(ac);
		return false;
	}
	fclose(ac);

	*charging = state == AC_STATE_CHARGING ? true : false;
	return true;
}

bool
get_ram(int *ram_usage)
{
	FILE *meminfo;
	char buf[2048];
	long mem_total = -1, mem_avail = -1;

	meminfo = fopen("/proc/meminfo", "r");
	if (meminfo == NULL) {
		warn("unable to open /proc/meminfo");
		return false;
	}
	while (fgets(buf, sizeof(buf), meminfo)) {
		if (sscanf(buf, "MemTotal: %ld kB", &mem_total) == 1)
			continue;
		if (sscanf(buf, "MemAvailable: %ld kB", &mem_avail) == 1)
			continue;
		if (mem_total != -1 && mem_avail != -1)
			break; // Found everything.
	}
	fclose(meminfo);

	if (mem_total == -1 || mem_avail == -1) {
		warnx("unable to parse /proc/meminfo");
		return false;
	}
	if (mem_total == 0) { /* Ponitless? */
		warnx("MemTotal is zero, cannot calculate used RAM");
		return false;
	}

	*ram_usage = (int)(100L * (mem_total - mem_avail) / mem_total);
	return true;
}

bool
set_status(char *status)
{
#ifdef CONSOLE_OUTPUT
	puts(status);
#else
	Display *display = XOpenDisplay(NULL);

	if (display == NULL) {
		warnx("unable to connect to display (XOpenDisplay)");
		return false;
	}
	
	XStoreName(display, XDefaultRootWindow(display), status);
	XSync(display, false);
	XCloseDisplay(display);
#endif
	return true;
}

[[noreturn]] int
main(void) {
	char load_buf[LOAD_LEN];
	char time_buf[TIME_LEN];
	char status_buf[STATUS_LEN];
	char ac_icon[2];
	int  ram_usage;
	int  batt_charge;
	bool charging = false; /* Pointless init, hides GCC warning. */
#ifdef STATUS_ANIMATION
	unsigned int curr_frame = 0; /* Current animation frame. */
#endif /* STATUS_ANIMATION */
	bool ok;

	if (get_charge(&batt_charge) == false)
		warnx("no battery was detected");

	for (;; sleep(status_delay)) {
		/* Clean-up, just in case. */
		memset(load_buf,   0, LOAD_LEN);
		memset(time_buf,   0, TIME_LEN);
		memset(status_buf, 0, STATUS_LEN);

		ok = get_load(load_buf);
		if (ok == false)
			errx(1, "unable to get CPU load");

		ok = get_time(time_buf);
		if (ok == false)
			errx(1, "unable to get date/time");

		ok = get_ram(&ram_usage);
		if (ok == false)
			errx(1, "unable to get available memory");

		/* Ensure string NULL-termination, just in case. */
		load_buf[LOAD_LEN-1]     = 0;
		time_buf[TIME_LEN-1]     = 0;
		status_buf[STATUS_LEN-1] = 0;

		if (get_charge(&batt_charge) == true) {
			/* Battery exists. */
			ok = is_charging(&charging);
			if (ok == false)
				errx(1, "unable to get charging status");
			ac_icon[0] = charging ? '+' : 0;
			ac_icon[1] = 0;

			snprintf(status_buf,
				STATUS_LEN,
				"%d%%%s%s%s%s%s%s[%d%%]"
#ifdef STATUS_ANIMATION
				" %c"
#endif /* STATUS_ANIMATION */
				, ram_usage,
				status_delim,
				load_buf,
				status_delim,
				time_buf,
				status_delim,
				ac_icon,
				batt_charge
#ifdef STATUS_ANIMATION
				, status_frames[curr_frame++]
#endif /* STATUS_ANIMATION */
				);
		} else {
			/* No battery. */
			snprintf(status_buf,
				STATUS_LEN,
				"%d%%%s%s%s%s"
#ifdef STATUS_ANIMATION
				" %c"
#endif /* STATUS_ANIMATION */
				, ram_usage,
				status_delim,
				load_buf,
				status_delim,
				time_buf
#ifdef STATUS_ANIMATION
				, status_frames[curr_frame++]
#endif /* STATUS_ANIMATION */
				);
		}

		ok = set_status(status_buf);
		if (ok == false)
			errx(1, "unable to set dwm status");

#ifdef STATUS_ANIMATION
		if (curr_frame >= N_FRAMES)
			curr_frame = 0; /* Animation reset. */
#endif /* STATUS_ANIMATION */
	}
}
