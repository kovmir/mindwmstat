/* Includes */
#include <err.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#ifndef CONSOLE_OUTPUT
#include <X11/Xlib.h>
#endif /* CONSOLE_OUTPUT */

/* Constants and macros */
#define LOAD_LEN 15
#define STATUS_LEN 95
#define TIME_LEN 30

/* Function prototypes */
/* Get average CPU load; return true on success. */
static bool get_load(char *buf);
/* Get local time; return true on success. */
static bool get_time(char *buf);
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
#ifdef STATUS_ANIMATION
	unsigned int curr_frame = 0; /* Current animation frame. */
#endif /* STATUS_ANIMATION */
	bool ok;

	for (;; sleep(status_delay)) {
		ok = get_load(load_buf);
		if (ok == false)
			errx(1, "unable to get CPU load");

		ok = get_time(time_buf);
		if (ok == false)
			errx(1, "unable to get date/time");

		snprintf(status_buf,
			STATUS_LEN,
			"%s%s%s "
#ifdef STATUS_ANIMATION
			"%c"
#endif /* STATUS_ANIMATION */
			, load_buf,
			status_delim,
			time_buf
#ifdef STATUS_ANIMATION
			, status_frames[curr_frame++]
#endif /* STATUS_ANIMATION */
			);

		ok = set_status(status_buf);
		if (ok == false)
			errx(1, "unable to set dwm status");

#ifdef STATUS_ANIMATION
		if (curr_frame >= N_FRAMES)
			curr_frame = 0; /* Animation reset. */
#endif /* STATUS_ANIMATION */
	}
}
