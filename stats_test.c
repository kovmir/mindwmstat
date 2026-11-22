/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifdef NDEBUG
#error "Enable assert() for unit tests."
#endif
#include <assert.h>

#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "stats.h"

#define MKSTEMP_TEMPLATE "/tmp/mindwmstat_XXXXXX"

static FILE *mk_meminfo(const char *content);
void test_load_query(void);
void test_load_query_small_buf(void);
void test_time_query(void);
void test_time_query_exact_buf(void);
void test_time_query_small_buf(void);
void test_memory_query(void);
void test_memory_query_zero_total(void);
void test_memory_query_missing_total(void);
void test_memory_query_missing_available(void);
void test_memory_query_reversed_order(void);
void test_memory_query_no_trailing_newline(void);
void test_memory_query_rounding(void);
void test_memory_query_large_values(void);
void test_memory_query_available_exceeds_total(void);

/* Open an in-memory "/proc/meminfo" for memory_query(). */
static FILE *
mk_meminfo(const char *content)
{
	return fmemopen((void *)content, strlen(content), "r");
}

void
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

void
test_load_query_small_buf(void)
{
	char buf[1];

	/* No room even for a NUL-terminated empty string: snprintf
	 * truncates, but the call still succeeds. */
	memset(buf, 0xAA, sizeof(buf));
	assert(load_query(buf, sizeof(buf)) == 0);
	assert(buf[0] == '\0');
}

void
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

void
test_time_query_exact_buf(void)
{
	char buf[20]; /* Exactly the formatted time plus a NUL byte. */

	setenv("TZ", "UTC", 1);
	tzset();
	memset(buf, 0xAA, sizeof(buf));
	assert(time_query(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 0) == 0);
	assert(strcmp(buf, "1970-01-01 00:00:00") == 0);
}

void
test_time_query_small_buf(void)
{
	char buf[4];

	setenv("TZ", "UTC", 1);
	tzset();
	/* Buffer too small: strftime() yields 0, so the call fails. */
	assert(time_query(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", 0) == -1);
}

void
test_memory_query(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nWhatEver: 666TB\nMemAvailable: 50 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

void
test_memory_query_zero_total(void)
{
	char meminfo_buf[] = "MemTotal: 0 kB\nMemAvailable: 0 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	/* Division by zero guard. */
	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

void
test_memory_query_missing_total(void)
{
	char meminfo_buf[] = "MemAvailable: 50 kB\nWhatEver: 666TB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

void
test_memory_query_missing_available(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nWhatEver: 666TB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == -1);

	assert(fclose(meminfo_file) == 0);
}

void
test_memory_query_reversed_order(void)
{
	char meminfo_buf[] = "MemAvailable: 50 kB\nMemTotal: 100 kB\n";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

void
test_memory_query_no_trailing_newline(void)
{
	char meminfo_buf[] = "MemTotal: 100 kB\nMemAvailable: 50 kB";
	FILE *meminfo_file = mk_meminfo(meminfo_buf);
	int used_mem;

	assert(memory_query(&used_mem, meminfo_file) == 0);
	assert(used_mem == 50);

	assert(fclose(meminfo_file) == 0);
}

void
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

void
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

void
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
