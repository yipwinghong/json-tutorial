#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "leptjson.h"

/**
 * TDD（测试驱动开发），主要循环步骤：
 *      1. 加入一个测试。
 *      2. 运行所有测试，新的测试应该会失败。
 *      3. 编写实现代码。
 *      4. 运行所有测试，若有测试失败回到3。
 *      5. 重构代码。
 *      6. 回到 1。
 * 在这个测试框架中使用了编译器提供的宏 __LINE__，代表编译时该行的行号；
 * 如果用函数或内联函数，每次的行号便都会相同，无法定位到具体报错的行，因此使用宏代替函数。
 */

/* main 方法返回结果 */
static int main_ret = 0;

/* 执行测试次数 */
static int test_count = 0;

/* 通过测试次数 */
static int test_pass = 0;

/* 测试错误输入 */
#define TEST_ERROR(error, json) \
    do {\
        lept_value v;\
        v.type = LEPT_FALSE;\
        EXPECT_EQ_INT(error, lept_parse(&v, json));\
        EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));\
    } while(0)

/* 测试判断方法 */
#define EXPECT_EQ_BASE(equality, expect, actual, format) \
    do {\
        test_count++;\
        if (equality) {\
            test_pass++;\
        } else {\
            fprintf(stderr, "%s:%d: expect: " format " actual: " format "\n", __FILE__, __LINE__, expect, actual);\
            main_ret = 1;\
        }\
    } while(0)

#define EXPECT_EQ_INT(expect, actual) \
    EXPECT_EQ_BASE((expect) == (actual), expect, actual, "%d")

/**
 * 测试解析 NULL
 */
static void test_parse_null() {
    lept_value v;
    v.type = LEPT_TRUE;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "null"));
    EXPECT_EQ_INT(LEPT_NULL, lept_get_type(&v));
}

/**
 * 测试解析 TRUE
 */
static void test_parse_true() {
    lept_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "true"));
    EXPECT_EQ_INT(LEPT_TRUE, lept_get_type(&v));
}

/**
 * 测试解析 FALSE
 */
static void test_parse_false() {
    lept_value v;
    v.type = LEPT_NULL;
    EXPECT_EQ_INT(LEPT_PARSE_OK, lept_parse(&v, "false"));
    EXPECT_EQ_INT(LEPT_FALSE, lept_get_type(&v));
}

/**
 * 测试解析空值
 */
static void test_parse_expect_value() {
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, "");
    TEST_ERROR(LEPT_PARSE_EXPECT_VALUE, " ");
}

/**
 * 测试非法值
 */
static void test_parse_invalid_value() {
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "nul");
    TEST_ERROR(LEPT_PARSE_INVALID_VALUE, "?");
}

static void test_parse_root_not_singular() {
    TEST_ERROR(LEPT_PARSE_ROOT_NOT_SINGULAR, "null x");
}

static void test_parse() {
    test_parse_null();
    test_parse_true();
    test_parse_false();
    test_parse_expect_value();
    test_parse_invalid_value();
    test_parse_root_not_singular();
}

int main() {
    test_parse();
    printf("%d/%d (%3.2f%%) passed\n", test_pass, test_count, test_pass * 100.0 / test_count);
    return main_ret;
}
