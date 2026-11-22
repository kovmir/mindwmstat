#ifndef STATS_TEST_H
#define STATS_TEST_H

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

#endif /* STATS_TEST_H */
