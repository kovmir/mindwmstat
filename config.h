/* Status elements delimiter. */
static const char *status_delim = " | ";

/* Status refresh animation frames; indicates whether the status is alive. */
#ifdef STATUS_ANIMATION
#define N_FRAMES 4
static char status_frames[N_FRAMES] = {'|', '/', '-', '\\'};
#endif /* STATUS_ANIMATION */

/* Status (and thus animation) update interval in seconds. */
static const int status_delay = 1;

/* Time format. */
static const char *time_format = "%F %a %R";

/* Battery. */
static const char *batt_path = "/sys/class/power_supply/BAT0/capacity";
/* AC power plug. */
static const char *ac_path = "/sys/class/power_supply/AC/online";
