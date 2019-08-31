#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <math.h>    /* HUGE_VAL */
#include <errno.h>   /* errno, ERANGE */

#define IS_DIGIT(ch) ((ch) >= '0' && (ch) <= '9')
#define IS_DIGIT_1TO9(ch) ((ch) >= '1' && (ch) <= '9')

#define EXPECT(c, ch)\
    do { assert(*c->json == (ch)); c->json++; } while(0)

typedef struct {
    const char *json;
} lept_context;

static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

/**
 * 解析 true/false/null
 *
 * @param c
 * @param v
 * @param literal
 * @param type
 * @return
 */
static int lept_parse_literal(lept_context *c, lept_value *v, const char* literal, lept_type type) {
#if 0
    size_t i;
    EXPECT(c, literal[0]);
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1])
            return LEPT_PARSE_INVALID_VALUE;
    c->json += i;
    v->type = type;
    return LEPT_PARSE_OK;
#endif

    char *p = literal;
    while(*p != '\0') {
        if (*p != c->json[0]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
        p++;
    }
    v->type = type;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context *c, lept_value *v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context *c, lept_value *v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_null(lept_context *c, lept_value *v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l')
        return LEPT_PARSE_INVALID_VALUE;
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

/**
 * 转换数值（把十进制的数字字符转换成二进制的 double）
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_number(lept_context *c, lept_value *v) {
    const char *p = c->json;

    /* 判断负号 */
    if (*p == '-') {
        p++;
    }

    /* 判断前置 0 */
    if (*p == '0') {
        p++;
    }

    /* 判断数字 */
    else {
        if (!IS_DIGIT_1TO9(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (p++; IS_DIGIT(*p); p++);
    }

    /* 判断小数点 */
    if (*p == '.') {
        p++;
        if (!IS_DIGIT(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (p++; IS_DIGIT(*p); p++);
    }

    /* 判断指数 */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') {
            p++;
        }
        if (!IS_DIGIT(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (p++; IS_DIGIT(*p); p++);
    }

    /* \TODO validate number */
    /**
     * 解析字符串 c->json，数值部分返回为 v->n、剩余字符串部分写入 end
     */
    char *end;
    v->n = strtod(c->json, &end);

    /* 判断 v->n 是否过大 */
    errno = 0;
    if (ERANGE && (v->n == HUGE_VAL || v->n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    c->json = p;
#if 0
    if (c->json == end) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json = end;
#endif
    v->type = LEPT_NUMBER;
    return LEPT_PARSE_OK;
}

/**
 * 解析 JSON 的入口函数
 * 对于 bool、null，只需要根据首字母选择具体的解析函数，
 * 对于 number，则要校验值是否正确
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_value(lept_context *c, lept_value *v) {
    switch (*c->json) {
        case 't':
            return lept_parse_literal(c, v, "true", LEPT_TRUE);
        case 'f':
            return lept_parse_literal(c, v, "false", LEPT_FALSE);
        case 'n':
            return lept_parse_literal(c, v, "null", LEPT_NULL);
        default:
            return lept_parse_number(c, v);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            v->type = LEPT_NULL;
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}

double lept_get_number(const lept_value *v) {

    /**
     * 当且仅当值非空、且值类型为 NUMBER 时才返回
     */
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->n;
}
