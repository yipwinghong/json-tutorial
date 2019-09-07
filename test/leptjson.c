#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <stdio.h>

/**
 * JSON 解析上下文，存放待解析的 JSON 字符串，用于解析时传参
 */
typedef struct {
    const char *json;
} lept_context;

/*
 * 判断 c 的首字母是否 ch，并向后移动一位
 */
#define EXPECT(c, ch) \
    do {\
        assert(*c->json == (ch));\
        c->json++;\
    } while(0)

/**
 * 解析空字符，包括空格、制表符、换行符等，把 c-> json 定位到首个非空字符出现的位置
 *
 * @param c
 */
static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    c->json = p;
}

/**
 * 解析 NULL
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_null(lept_context *c, lept_value *v) {
    EXPECT(c, 'n');
    if (c->json[0] != 'u' || c->json[1] != 'l' || c->json[2] != 'l') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

/**
 * 解析 TRUE
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_true(lept_context *c, lept_value *v) {
    EXPECT(c, 't');
    if (c->json[0] != 'r' || c->json[1] != 'u' || c->json[2] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 3;
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context *c, lept_value *v) {
    EXPECT(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

/**
 * 解析非空 JSON 值
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_value(lept_context *c, lept_value *v) {

    /* 根据字符串首字符选择具体的解析方法 */
    switch (*c->json) {
        case 'n':
            return lept_parse_null(c, v);
        case 't':
            return lept_parse_true(c, v);
        case 'f':
            return lept_parse_false(c, v);
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        default:
            return LEPT_PARSE_INVALID_VALUE;
    }
}

/**
 * 解析 JSON 字符串，入口函数
 *
 * @param v
 * @param json
 * @return
 */
int lept_parse(lept_value *v, const char *json) {
    int ret;
    lept_context c;
    assert(v != NULL);

    /* 结构体的域以“.”访问 */
    c.json = json;

    /* 结构体指针的域以“->”访问 */
    v->type = LEPT_NULL;

    lept_parse_whitespace(&c);
    if ((ret = lept_parse_value(&c, v)) == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*c.json != '\0') {
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

/**
 * 获取 JSON 值
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}
