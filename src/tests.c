#include "tests.h"

TEST q_atoi_decimal(void) {
    ASSERT_EQ(q_atoi("46"), 46);
    ASSERT_EQ(q_atoi("-873"), -873);
    return 0;
}

TEST q_atoi_hex(void) {
    ASSERT_EQ(q_atoi("0xBCA4"), 0xbca4);
    return 0;
}

SUITE(test_atoi) {
    RUN_TEST(q_atoi_decimal);
    RUN_TEST(q_atoi_hex);
}

/* Add definitions that need to be in the test runner's main file. */
GREATEST_MAIN_DEFS();

int run_tests(void) {
    GREATEST_INIT();
    RUN_SUITE(test_atoi);
    GREATEST_PRINT_REPORT();
    return greatest_all_passed();
}