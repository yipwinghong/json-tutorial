#ifdef _WINDOWS
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#endif

#include "leptjson.h"
#include <assert.h>     /* assert() */
#include <stdlib.h>     /* NULL, strtod() */
#include <stdio.h>
#include <stddef.h>     /* size_t */
#include <math.h>       /* HUGE_VAL */
#include <errno.h>      /* errno, ERANGE */
#include <string.h>     /* memcpy() */

/* 缓冲区堆栈大小（好处是使用者可在编译选项中自行设置宏，没设置的话就用缺省值） */
#ifndef LEPT_PARSE_STACK_INIT_SIZE
#define LEPT_PARSE_STACK_INIT_SIZE 256
#endif

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
 * 入栈
 */
#define PUTC(c, ch)\
    do {\
        *(char*) lept_context_push(c, sizeof(char)) = (ch);\
    } while(0)

/**
 * JSON 解析上下文，存放待解析的 JSON 字符串，用于解析时传参
 */
typedef struct {
    const char *json;       /* 待解析的 JSON 字符串 */
    char *stack;            /* 缓冲区（堆栈），把解析的结果先储存在一个临时的缓冲区（以字节存储），最后再用 lept_set_string() 把结果设进值之中 */
    size_t size, top;       /* 堆栈容量和当前栈顶位置，此处使用动态堆栈实现，避免每次解析字符串都要新建一个 */
} lept_context;

/**
 * 初始化 JSON 对象
 */
#define lept_init(v) \
    do {\
        (v)->type = LEPT_NULL; \
    } while(0)

static int lept_parse_value(lept_context *c, lept_value *v); /* 前向声明 */

/**
 * JSON 上下文入栈
 *
 * @param c
 * @param size
 * @return
 */
static void *lept_context_push(lept_context *c, size_t size) {
    void *ret;
    assert(size > 0);
    /* 栈容量不足，需要扩容 */
    if (c->top + size >= c->size) {
        /* 第一次扩容，设置为初始容量 */
        if (c->size == 0) {
            c->size = LEPT_PARSE_STACK_INIT_SIZE;
        }
        /* 每次扩容 1.5 倍，直到正好满足入栈要求 */
        while (c->top + size >= c->size) {
            c->size += c->size >> 1;
        }
        /* 分配内存（尝试重新调整之前调用 malloc 或 calloc 所分配的 ptr 所指向的内存块的大小） */
        /* c->stack 在初始化时为 NULL，realloc(NULL, size) 的行为是等价于 malloc(size) 的 */
        c->stack = (char *) realloc(c->stack, c->size);
    }
    /* 返回当前入栈数据数据起始位置的指针 */
    ret = c->stack + c->top;
    c->top += size;
    return ret;
}

/**
 * JSON 上下文出栈
 *
 * @param c
 * @param size
 * @return
 */
static void *lept_context_pop(lept_context *c, size_t size) {
    assert(c->top >= size);
    return c->stack + (c->top -= size);
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
 * 解析数值（依次判断负号、前置0、小数点、指数）
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
    v->u.n = strtod(c->json, NULL);

    /* 表示溢出的双精度浮点数 */
    if (errno == ERANGE && (v->u.n == HUGE_VAL || v->u.n == -HUGE_VAL)) {
        return LEPT_PARSE_NUMBER_TOO_BIG;
    }
    v->type = LEPT_NUMBER;
    c->json = p;
    return LEPT_PARSE_OK;
}

static const char *lept_parse_hex4(const char *p, unsigned *u) {
    int i;
    *u = 0;
    for (i = 0; i < 4; i++) {
        char ch = *p++;
        *u <<= 4;
        if (ch >= '0' && ch <= '9') *u |= ch - '0';
        else if (ch >= 'A' && ch <= 'F') *u |= ch - ('A' - 10);
        else if (ch >= 'a' && ch <= 'f') *u |= ch - ('a' - 10);
        else return NULL;
    }
    return p;
}

static void lept_encode_utf8(lept_context *c, unsigned u) {
    if (u <= 0x7F)
        PUTC(c, u & 0xFF);
    else if (u <= 0x7FF) {
        PUTC(c, 0xC0 | ((u >> 6) & 0xFF));
        PUTC(c, 0x80 | (u & 0x3F));
    } else if (u <= 0xFFFF) {
        PUTC(c, 0xE0 | ((u >> 12) & 0xFF));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    } else {
        assert(u <= 0x10FFFF);
        PUTC(c, 0xF0 | ((u >> 18) & 0xFF));
        PUTC(c, 0x80 | ((u >> 12) & 0x3F));
        PUTC(c, 0x80 | ((u >> 6) & 0x3F));
        PUTC(c, 0x80 | (u & 0x3F));
    }
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

/**
 * 解析字符串
 *
 * @param c
 * @param v
 * @return
 */
static int lept_parse_string(lept_context *c, lept_value *v) {
    /* 备份栈顶*/
    size_t head = c->top, len;
    const char *p;
    unsigned u, u2;

    /* 字符串以 “"” 开始，判断并去除第一个 “"” */
    EXPECT(c, '\"');
    p = c->json;
    for (;;) {
        char ch = *p++;
        switch (ch) {
            case '\"':
                /* 字符串以 “"” 结束，此时计算串长度、把入栈字符出栈 */
                len = c->top - head;
                lept_set_string(v, (const char *) lept_context_pop(c, len), len);
                c->json = p;
                return LEPT_PARSE_OK;
            case '\0':
                c->top = head;
                return LEPT_PARSE_MISS_QUOTATION_MARK;
            case '\\':
                /* 处理转义字符，对“\”的下一个字符判断 */
                switch (*p++) {
                    case '\"':
                        PUTC(c, '\"');
                        break;
                    case '\\':
                        PUTC(c, '\\');
                        break;
                    case '/':
                        PUTC(c, '/');
                        break;
                    case 'b':
                        PUTC(c, '\b');
                        break;
                    case 'f':
                        PUTC(c, '\f');
                        break;
                    case 'n':
                        PUTC(c, '\n');
                        break;
                    case 'r':
                        PUTC(c, '\r');
                        break;
                    case 't':
                        PUTC(c, '\t');
                        break;
                    case 'u':
                        if (!(p = lept_parse_hex4(p, &u)))
                            STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                        if (u >= 0xD800 && u <= 0xDBFF) { /* surrogate pair */
                            if (*p++ != '\\')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (*p++ != 'u')
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            if (!(p = lept_parse_hex4(p, &u2)))
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_HEX);
                            if (u2 < 0xDC00 || u2 > 0xDFFF)
                                STRING_ERROR(LEPT_PARSE_INVALID_UNICODE_SURROGATE);
                            u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;
                        }
                        lept_encode_utf8(c, u);
                        break;
                    default:
                        c->top = head;
                        return LEPT_PARSE_INVALID_STRING_ESCAPE;
                }
                break;
            default:
                /* 处理不合法字符 */
                if ((unsigned char) ch < 0x20) {
                    c->top = head;
                    return LEPT_PARSE_INVALID_STRING_CHAR;
                }

                /* 待解析字符入栈 */
                PUTC(c, ch);
        }
    }
}

static int lept_parse_array(lept_context *c, lept_value *v) {
    size_t size = 0;
    int ret;
    EXPECT(c, '[');
    if (*c->json == ']') {
        c->json++;
        v->type = LEPT_ARRAY;
        v->u.a.size = 0;
        v->u.a.e = NULL;
        return LEPT_PARSE_OK;
    }
    for (;;) {
        lept_value e;
        lept_init(&e);
        if ((ret = lept_parse_value(c, &e)) != LEPT_PARSE_OK) {
            return ret;
        }
        memcpy(lept_context_push(c, sizeof(lept_value)), &e, sizeof(lept_value));
        size++;
        if (*c->json == ',') {
            c->json++;
        } else if (*c->json == ']') {
            c->json++;
            v->type = LEPT_ARRAY;
            v->u.a.size = size;
            size *= sizeof(lept_value);
            memcpy(v->u.a.e = (lept_value*) malloc(size), lept_context_pop(c, size), size);
        }
    }

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
        case '"':
            return lept_parse_string(c, v);
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

    /* 最后确定上下文堆栈清空，表示解析完成，并释放栈内存 */
    assert(c.top == 0);
    free(c.stack);
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
 * 设置数值
 *
 * @param v
 * @param n
 */
void lept_set_number(lept_value *v, double n) {
    lept_free(v);
    v->u.n = n;
    v->type = LEPT_NUMBER;
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
 * 获取字符串值
 *
 * @param v
 * @return
 */
const char *lept_get_string(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_STRING);
    return v->u.s.s;
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

    /* 把 s 中的前 len 个字符复制给 v->u.s.s，设置长度，末尾补"\0" */
    memcpy(v->u.s.s, s, len);
    v->u.s.s[len] = '\0';
    v->u.s.len = len;
    v->type = LEPT_STRING;
}

/**
 * 获取布尔值
 *
 * @param v
 * @return
 */
int lept_get_boolean(const lept_value *v) {
    assert(v != NULL && (v->type == LEPT_TRUE || v->type == LEPT_FALSE));
    return v->type == LEPT_TRUE;
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

/**
 * 获取数组大小
 *
 * @param v
 * @return
 */
size_t lept_get_array_size(const lept_value *v) {
    assert(v != NULL && v->type == LEPT_ARRAY);
    return v->u.a.size;
}

/**
 * 获取数组元素
 *
 * @param v
 * @param index
 * @return
 */
lept_value *lept_get_array_element(const lept_value *v, size_t index) {
    assert(v != NULL && v->type == LEPT_ARRAY && index < v->u.a.size);
    return &v->u.a.e[index];
}
