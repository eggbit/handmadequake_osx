#include "greatest.h"
#include "quakedef.h"

TEST q_atoi_decimal(void) {
    ASSERT_EQ(q_atoi("46"), 46);
    ASSERT_EQ(q_atoi("-873"), -83);
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

GREATEST_MAIN_DEFS();

int main(int argc, char * argv[]) {
    GREATEST_MAIN_BEGIN();
    RUN_SUITE(test_atoi);
    GREATEST_MAIN_END();
}
