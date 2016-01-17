#include "greatest.h"
#include "quakedef.h"

//
// q_atoi
//
TEST q_atoi_positive_integer(void) {
    ASSERT_EQ(46, q_atoi("46"));
    PASS();
}

TEST q_atoi_negative_integer(void) {
    ASSERT_EQ(-873, q_atoi("-873"));
    PASS();
}

TEST q_atoi_negative_float(void) {
    ASSERT_EQ(-87, q_atoi("-87.3"));
    PASS();
}

TEST q_atoi_positive_float(void) {
    ASSERT_EQ(923, q_atoi("923.673"));
    PASS();
}

TEST q_atoi_hex_normal(void) {
    ASSERT_EQ(0xbca4, q_atoi("0xBCA4"));
    PASS();
}

TEST q_atoi_hex_mixed(void) {
    ASSERT_EQ(0xcdffca, q_atoi("0xcDfFcA"));
    PASS();
}

TEST q_atoi_hex_negative(void) {
    ASSERT_EQ(0, q_atoi("-0xBCA4"));
    PASS();
}

TEST q_atoi_hex_double(void) {
    ASSERT_EQ(0, q_atoi("0xBC0xCD"));
    PASS();
}

SUITE(test_q_atoi) {
    RUN_TEST(q_atoi_positive_integer);
    RUN_TEST(q_atoi_negative_integer);
    RUN_TEST(q_atoi_negative_float);
    RUN_TEST(q_atoi_positive_float);

    RUN_TEST(q_atoi_hex_normal);
    RUN_TEST(q_atoi_hex_mixed);
    RUN_TEST(q_atoi_hex_negative);
    RUN_TEST(q_atoi_hex_double);
}

//
// q_strcmp
//
TEST q_strcmp_equal(void) {
    ASSERT_EQ(0, q_strcmp("hello", "hello"));
    PASS();
}

TEST q_strcmp_left_earlier(void) {
    ASSERT_EQ(1, q_strcmp("abc", "bcd"));
    PASS();
}

TEST q_strcmp_right_earlier(void) {
    ASSERT_EQ(-1, q_strcmp("cde", "abc"));
    PASS();
}

TEST q_strcmp_null_check_right(void) {
    ASSERT_FALSE(q_strcmp("hello", NULL));
    PASS();
}

TEST q_strcmp_null_check_left(void) {
    ASSERT_FALSE(q_strcmp(NULL, "hello"));
    PASS();
}

TEST q_strcmp_null_double(void) {
    ASSERT_FALSE(q_strcmp(NULL, NULL));
    PASS();
}

SUITE(test_q_strcmp) {
    RUN_TEST(q_strcmp_equal);
    RUN_TEST(q_strcmp_left_earlier);
    RUN_TEST(q_strcmp_right_earlier);
    RUN_TEST(q_strcmp_null_check_right);
    RUN_TEST(q_strcmp_null_check_left);
    RUN_TEST(q_strcmp_null_double);

}

//
// q_strcpy
//
TEST q_strcpy_equal_array(void) {
    char new[40] = {0};
    q_strcpy(new, "Hello, world!");

    ASSERT_STR_EQ("Hello, world!", new);
    PASS();
}

TEST q_strcpy_null_check_right(void) {
    char *new = "";
    q_strcpy(new, NULL);

    ASSERT_STR_EQ("", new);
    PASS();
}

SUITE(test_q_strcpy) {
    RUN_TEST(q_strcpy_equal_array);
    RUN_TEST(q_strcpy_null_check_right);
}

//
// q_strncpy
//
TEST q_strncpy_partial_copy(void) {
    char new[40] = {0};
    q_strncpy(new, "Hello, world!", 3);

    ASSERT_STR_EQ("Hel", new);
    PASS();
}

TEST q_strncpy_full_copy(void) {
    char new[40] = {0};
    q_strncpy(new, "Hello, world!", 13);

    ASSERT_STR_EQ("Hello, world!", new);
    PASS();
}

TEST q_strncpy_overflow_copy(void) {
    char new[40] = {0};
    q_strncpy(new, "Hello, world!", 50);

    ASSERT_STR_EQ("Hello, world!", new);
    PASS();
}

TEST q_strncpy_null_check(void) {
    char new[40] = {0};
    q_strncpy(new, NULL, 3);

    ASSERT_STR_EQ("", new);
    PASS();
}

TEST q_strncpy_zero_count_check(void) {
    char new[40] = {0};
    q_strncpy(new, "Hello!", 0);

    ASSERT_STR_EQ("", new);
    PASS();
}

TEST q_strncpy_negative_count_check(void) {
    char new[40] = {0};
    q_strncpy(new, "Hello!", -1);

    ASSERT_STR_EQ("", new);
    PASS();
}

SUITE(test_q_strncpy) {
    RUN_TEST(q_strncpy_partial_copy);
    RUN_TEST(q_strncpy_full_copy);
    RUN_TEST(q_strncpy_overflow_copy);
    RUN_TEST(q_strncpy_null_check);
    RUN_TEST(q_strncpy_zero_count_check);
    RUN_TEST(q_strncpy_negative_count_check);
}

//
// q_strlen_f
//
TEST q_strlen_size(void) {
    ASSERT_EQ(5, q_strlen("Hello"));
    PASS();
}

TEST q_strlen_null_check(void) {
    ASSERT_FALSE(q_strlen(NULL));
    PASS();
}

TEST q_strlen_empty_check(void) {
    ASSERT_EQ(0, q_strlen(""));
    PASS();
}

SUITE(test_q_strlen) {
    RUN_TEST(q_strlen_size);
    RUN_TEST(q_strlen_null_check);
    RUN_TEST(q_strlen_empty_check);
}

//
// com_check_parm
//
TEST com_check_parm_string(void) {
    const char *argv[] = {"", "-width", "640", "-height", "480", "-setalpha", "0x4cb"};
    int32_t args = sizeof(argv) / sizeof(argv[0]);

    ASSERT_STR_EQ("480", com_check_parm("-height", args, argv));
    ASSERT_STR_EQ("640", com_check_parm("-width", args, argv));
    ASSERT_STR_EQ("0x4cb", com_check_parm("-setalpha", args, argv));
    PASS();
}

TEST com_check_parm_bool(void) {
    const char *argv[] = {"", "-startwindowed", "-setalpha", "50"};
    int32_t args = sizeof(argv) / sizeof(argv[0]);

    ASSERT_STR_EQ("1", com_check_parm("-startwindowed", args, argv));
    PASS();
}

TEST com_check_parm_bool_at_end(void) {
    const char *argv[] = {"", "-startwindowed"};
    int32_t args = sizeof(argv) / sizeof(argv[0]);

    ASSERT_STR_EQ("1", com_check_parm("-startwindowed", args, argv));
    PASS();
}

TEST com_check_parm_not_found(void) {
    const char *argv[] = {"", "-width", "640"};
    int32_t args = sizeof(argv) / sizeof(argv[0]);

    ASSERT_FALSE(com_check_parm("-xyz", args, argv));
    PASS();
}

TEST com_check_parm_no_argv(void) {
    const char *argv[] = {""};
    int32_t args = sizeof(argv) / sizeof(argv[0]);

    ASSERT_FALSE(com_check_parm("-height", args, argv));
    PASS();
}

SUITE(test_com_check_parm) {
    RUN_TEST(com_check_parm_string);
    RUN_TEST(com_check_parm_bool);
    RUN_TEST(com_check_parm_bool_at_end);
    RUN_TEST(com_check_parm_not_found);
    RUN_TEST(com_check_parm_no_argv);
}


//
// main
//
GREATEST_MAIN_DEFS();

int main(int argc, char * argv[]) {
    GREATEST_MAIN_BEGIN();

    RUN_SUITE(test_q_atoi);
    RUN_SUITE(test_q_strcmp);
    RUN_SUITE(test_q_strcpy);
    RUN_SUITE(test_q_strncpy);
    RUN_SUITE(test_q_strlen);
    RUN_SUITE(test_com_check_parm);

    GREATEST_MAIN_END();
}
