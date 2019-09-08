#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL, strtod() */
#include <stdio.h>
#include <math.h>    /* HUGE_VAL */
#include <errno.h>   /* errno, ERANGE */
#include <string.h>  /* memcpy() */

/**
 * JSON 解析上下文，存放待解析的 JSON 字符串，用于解析时传参
 */
typedef struct {
    const char *json;       /* 待解析的 JSON 字符串 */
    char *stack;            /* 缓冲区（堆栈），把解析的结果先储存在一个临时的缓冲区，最后再用 lept_set_string() 把结果设进值之中 */
    size_t size, top;       /* 堆栈容量和顶部指针 */
} lept_context;

#define lept_init(v) \
    do {\
        (v)->type = LEPT_NULL; \
    } while(0)

/**
 * 判断 c 的首字母是否 ch，并向后移动一位
 */
#define EXPECT(c, ch) \
    do {\
        assert(*c->json == (ch));\
        c->json++;\
    } while(0)

/**
 * 判断数字
 */
#define ISDIGIT(ch)         ((ch) >= '0' && (ch) <= '9')

/**
 * 判断 1~9 数字
 */
#define ISDIGIT1TO9(ch)     ((ch) >= '1' && (ch) <= '9')

/**
 * 解析 NULL、TRUE、FALSE
 *
 * @param c
 * @param v
 * @param literal
 * @param type
 * @return
 */
static int lept_parse_literal(lept_context *c, lept_value *v, const char *literal, lept_type type) {
#if 0
    /* 在 C 语言中，数组长度、索引值使用 size_t 类型，而不是 int 或 unsigned */
    size_t i;
    EXPECT(c, literal[0]);
    /* 结束条件：!literal[i + 1]，返回 0 退出循环 */
    for (i = 0; literal[i + 1]; i++)
        if (c->json[i] != literal[i + 1]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
    c->json += i;
#endif
    const char *p = literal;
    while (*p != '\0') {
        if (*p != c->json[0]) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
        p++;
    }
    v->type = type;
    return LEPT_PARSE_OK;
}

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
 * 解析数值
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_number(lept_context *c, lept_value *v) {
#if 0
    char *end;

    /* 解析字符串 c->json，数值部分返回为 v->n、剩余字符串部分写入 end */
    v->n = strtod(c->json, &end);
    if (c->json == end) {
        return LEPT_PARSE_INVALID_VALUE;
    }
    c->json = end;
#endif
    const char *p = c->json;

    /* 处理负号 */
    if (*p == '-') {
        p++;
    }

    /* 处理前置 0 */
    if (*p == '0') {
        p++;
    } else {
        /* 如果第一个数字不是 0、且不为 1-9 的数字，返回错误 */
        if (!ISDIGIT1TO9(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        /* 跳过连续的数字 */
        for (p++; ISDIGIT(*p); p++);
    }

    /* 处理小数点 */
    if (*p == '.') {
        p++;
        /* 如果小数点后不是数字，返回错误 */
        if (!ISDIGIT(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        /* 跳过连续的数字 */
        for (p++; ISDIGIT(*p); p++);
    }

    /* 处理指数（e+123） */
    if (*p == 'e' || *p == 'E') {
        p++;
        if (*p == '+' || *p == '-') {
            p++;
        }
        if (!ISDIGIT(*p)) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        for (p++; ISDIGIT(*p); p++);
    }
    errno = 0;
    v->u->n = strtod(c->json, NULL);
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->type = LEPT_NUMBER;
    c->json = p;
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

    /* 结构体的域以“.”访问，结构体指针的域以“->”访问 */

    /* 初始化 JSON 解析上下文 */
    c.json = json;
    c.stack = NULL;
    c.size = c.top = 0;

    /* 初始化 JSON 对象 */
    lept_init(v);

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
 * 释放内存（用于字符数组，需要判断类型、避免重复释放）
 *
 * @param v
 */
void lept_free(lept_value *v) {
    assert(v != NULL);
    if (v->type == LEPT_STRING) {
        free(v->u.s.s);
    }
    v->type = LEPT_NULL;
}

/**
 * 获取数值
 *
 * @param v
 * @return
 */
double lept_get_number(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_NUMBER);
    return v->u.n;
}

/**
 * 获取类型
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}

/**
 * 获取字符串长度
 *
 * @param v
 * @return
 */
size_t lept_get_string_length(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.len;
}

/**
 * 设置字符串值
 *
 * @param v
 * @param s
 * @param len
 */
void lept_set_string(lept_value *v, const char *s, size_t len) {
    /* 校验入参、释放内存 */
    assert(v != NULL && (s != NULL || len == 0));
    lept_free(v);

    /* 分配内存 */
    v->u.s.s = (char *) malloc(len + 1);

    /* 把 s 中的前 len 个字符复制给 v->u.s.s */
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

/**
 * 设置布尔值
 *
 * @param v
 * @param b
 */
void lept_set_boolean(lept_value *v, int b) {
    lept_free(v);
    v->type = b ? LEPT_TRUE : LEPT_FALSE;
}
