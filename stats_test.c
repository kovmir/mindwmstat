/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifdef NDEBUG
#error "Enable assert() for unit tests."
#endif
#include <assert.h>

#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stats.h"

#define MKSTEMP_TEMPLATE "/tmp/mindwmstat_XXXXXX"

static FILE *mk_meminfo(const char *content);
static void test_load_query(void);
static void test_load_query_small_buf(void);
static void test_time_query(void);
static void test_time_query_exact_buf(void);
static void test_time_query_small_buf(void);
static void test_battery_close(void);
static void test_battery_open(void);
static void test_battery_query(void);
static void test_memory_query(void);
static void test_memory_query_zero_total(void);
static void test_memory_query_missing_total(void);
static void test_memory_query_missing_available(void);
static void test_memory_query_reversed_order(void);
static void test_memory_query_no_trailing_newline(void);
static void test_memory_query_rounding(void);
static void test_memory_query_large_values(void);
static void test_memory_query_available_exceeds_total(void);

/* Open an in-memory "/proc/meminfo" for memory_query(). */
static FILE *
mk_meminfo(const char *content)
{
	return fmemopen((void *)content, strlen(content), "r");
}

static void
test_load_query(void)
{
	char buf[16];
	float a, b, c;
	int n_matches;

	memset(buf, 0xAA, sizeof(buf));
	assert(load_query(buf, sizeof(buf)) == 0);
	n_matches = sscanf(buf, "%f %f %f", &a, &b, &c);

	assert(n_matches == 3);
	assert(strlen(buf) < sizeof(buf));
}

static void
test_load_query_small_buf(void)
{
	char buf[1];

	/* No room even for a NUL-terminated empty string: snprintf
	 * truncates, but the call still succeeds. */
	memset(buf, 0xAA, sizeof(buf));
	assert(load_query(buf, sizeof(buf)) == 0);
	assert(buf[0] == '\0');
}

static void
test_time_query(void)
{
	char buf[64];

	setenv("TZ", "UTC", 1);
	tzset();
	memset(buf, 0xAA, sizeof(buf));
	assert(time_query(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 0) == 0);
	assert(strcmp(buf, "1970-01-01 00:00:00") == 0);
	assert(strlen(buf) < sizeof(buf));
}

static void
test_time_query_exact_buf(void)
{
	char buf[20]; /* Exactly the formatted time plus a NUL byte. */

	setenv("TZ", "UTC", 1);
	tzset();
	memset(buf, 0xAA, sizeof(buf));
	assert(time_query(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 0) == 0);
	assert(strcmp(buf, "1970-01-01 00:00:00") == 0);
}

static void
test_time_query_small_buf(void)
{
	char buf[4];

	setenv("TZ", "UTC", 1);
	tzset();
	/* Buffer too small: strftime() yields 0, so the call fails. */
	assert(time_query(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 0) == -1);
}

static void
test_battery_close(void)
{
	Battery batt;
	int cap_fd = open("/dev/null", O_RDONLY);
	int ac_fd = open("/dev/null", O_RDONLY);
	assert(cap_fd != -1);
	assert(ac_fd != -1);

	/* Unopened handle: nothing to close. */
	batt.cap_fd = -1;
	batt.ac_fd = -1;
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
	/* Only cap_fd valid: ac_fd is left untouched. */
	batt.cap_fd = cap_fd;
	batt.ac_fd = -1;
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
	assert(close(cap_fd) == -1 && errno == EBADF);
	/* Only ac_fd valid: cap_fd is left untouched. */
	batt.cap_fd = -1;
	batt.ac_fd = ac_fd;
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
	assert(close(ac_fd) == -1 && errno == EBADF);
	/* Both fds valid: both closed, double close is safe. */
	cap_fd = open("/dev/null", O_RDONLY);
	ac_fd = open("/dev/null", O_RDONLY);
	assert(cap_fd != -1);
	assert(ac_fd != -1);
	batt.cap_fd = cap_fd;
	batt.ac_fd = ac_fd;
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
	assert(close(cap_fd) == -1 && errno == EBADF);
	assert(close(ac_fd) == -1 && errno == EBADF);
	/* Double close: no-op. */
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
}

static void
test_battery_open(void)
{
	Battery batt;
	const char *bad_ac = "/nonexistent/online";
	const char *bad_cap = "/nonexistent/capacity";
	const char *failed_path = NULL;
	char cap_tmpl[] = MKSTEMP_TEMPLATE;
	char ac_tmpl[] = MKSTEMP_TEMPLATE;
	int fd;

	/* Create the backing files. */
	fd = mkstemp(cap_tmpl);
	assert(fd != -1);
	assert(close(fd) == 0);
	fd = mkstemp(ac_tmpl);
	assert(fd != -1);
	assert(close(fd) == 0);
	/* Success: both paths exist. */
	assert(battery_open(cap_tmpl, ac_tmpl, &batt, &failed_path) == 0);
	assert(failed_path == NULL); /* Untouched on success. */
	assert(batt.cap_fd != -1);
	assert(batt.ac_fd != -1);
	battery_close(&batt);
	assert(batt.cap_fd == -1);
	assert(batt.ac_fd == -1);
	/* Failure: missing battery path. */
	failed_path = NULL;
	assert(battery_open(bad_cap, ac_tmpl, &batt, &failed_path) == -1);
	assert(failed_path == bad_cap);
	/* Failure: NULL failed_path is accepted. */
	assert(battery_open(bad_cap, ac_tmpl, &batt, NULL) == -1);
	/* Failure: missing AC path. */
	failed_path = NULL;
	assert(battery_open(cap_tmpl, bad_ac, &batt, &failed_path) == -1);
	assert(failed_path == bad_ac);
	/* Clean up. */
	assert(unlink(cap_tmpl) == 0);
	assert(unlink(ac_tmpl) == 0);
}

static void
test_battery_query(void)
{
	Battery batt;
	BatteryInfo bi;
	char cap_content[] = "42";
	char ac_content[] = "1";
	char cap_tmpl[] = MKSTEMP_TEMPLATE;
	char ac_tmpl[] = MKSTEMP_TEMPLATE;
	int cap_fd = mkstemp(cap_tmpl);
	int ac_fd = mkstemp(ac_tmpl);
	assert(cap_fd != -1);
	assert(ac_fd != -1);
	batt.cap_fd = cap_fd;
	batt.ac_fd = ac_fd;
	/* Write test content. */
	assert(write(cap_fd, cap_content, strlen(cap_content)) != -1);
	assert(write(ac_fd, ac_content, strlen(ac_content)) != -1);
	/* Read battery status. */
	assert(battery_query(batt, &bi) == 0);
	assert(bi.capacity == 42);
	assert(bi.power_state == POWER_AC);
	/* Read again... */
	assert(battery_query(batt, &bi) == 0);
	assert(bi.capacity == 42);
	assert(bi.power_state == POWER_AC);
	/* Clean up. */
	assert(close(cap_fd) == 0);
	assert(close(ac_fd) == 0);
	assert(unlink(cap_tmpl) == 0);
	assert(unlink(ac_tmpl) == 0);
}

static void
test_memory_query(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nWhatEver: 666TB\nMemAvailable: 50 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_zero_total(void)
{
	char meminfo_buf[] = "MemTotal: 0 kB\nMemAvailable: 0 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	/* Division by zero guard. */
	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_missing_total(void)
{
	char meminfo_buf[] = "MemAvailable: 50 kB\nWhatEver: 666TB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_missing_available(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nWhatEver: 666TB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_reversed_order(void)
{
	char meminfo_buf[] = "MemAvailable: 50 kB\nMemTotal: 100 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_no_trailing_newline(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nMemAvailable: 50 kB";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_rounding(void)
{
	char meminfo_buf[] = "MemTotal: 300 kB\nMemAvailable: 101 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	/* 100 * 199 / 300 == 66.33..., truncated by integer division. */
	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 66);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_large_values(void)
{
	char meminfo_buf[] =
	    "MemTotal: 33554432 kB\nMemAvailable: 1048576 kB\n"; /* 32 GiB total. */
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	/* 100L * 32505856 exceeds 32-bit LONG_MAX, so this only passes
	 * where long is 64 bits (LP64). */
	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 96);

	assert(fclose(meminfo_file) == 0);
}

static void
test_memory_query_available_exceeds_total(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nMemAvailable: 150 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	/* Nonsensical input is not rejected: the result goes negative. */
	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == -50);

	assert(fclose(meminfo_file) == 0);
}

int
main(void)
{
	test_load_query();
	test_load_query_small_buf();
	test_time_query();
	test_time_query_exact_buf();
	test_time_query_small_buf();
	test_battery_open();
	test_battery_query();
	test_battery_close();
	test_memory_query();
	test_memory_query_zero_total();
	test_memory_query_missing_total();
	test_memory_query_missing_available();
	test_memory_query_reversed_order();
	test_memory_query_no_trailing_newline();
	test_memory_query_rounding();
	test_memory_query_large_values();
	test_memory_query_available_exceeds_total();
	return 0;
}
