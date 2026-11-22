/* SPDX-License-Identifier: MIT
 * Copyright (c) 2026 Ivan Kovmir */
#ifdef NDEBUG
#error "Enable assert() for unit tests."
#endif
#include <assert.h>

#include "stats_test.h"
#include "battery_test.h"

int
main(void)
{
	test_load_query();
	test_load_query_small_buf();
	test_time_query();
	test_time_query_exact_buf();
	test_time_query_small_buf();
	test_memory_query();
	test_memory_query_zero_total();
	test_memory_query_missing_total();
	test_memory_query_missing_available();
	test_memory_query_reversed_order();
	test_memory_query_no_trailing_newline();
	test_memory_query_rounding();
	test_memory_query_large_values();
	test_memory_query_available_exceeds_total();

	test_battery_close();
	test_battery_open();
	test_battery_query();
	return 0;
}
