/* Status elements delimiter. */
static const char *status_delim = " | ";

/* Time format. */
static const char *time_format = "%F %a %R"; /* See date(1) for format. */

/* Battery. */
static const char *batt_path = "/sys/class/power_supply/BAT0/capacity";
/* AC power plug. */
static const char *ac_path = "/sys/class/power_supply/AC/online";
/* Battery charge and discharge icons. */
static const char ac_charge_icon    = '+'; /* =0 no icon. */
static const char ac_discharge_icon = '-'; /* =0 no icon. */

/* Status refresh animation frames; indicates whether the status is alive. */
#ifdef STATUS_ANIMATION
#define N_FRAMES 4
static char status_frames[N_FRAMES] = {'|', '/', '-', '\\'};
#endif /* STATUS_ANIMATION */

/* Status (and thus animation) update interval in seconds. */
static const unsigned int status_delay = 1;
