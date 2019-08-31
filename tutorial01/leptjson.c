#include "leptjson.h"
#include <assert.h>  /* assert() */
#include <stdlib.h>  /* NULL */
#include <stdio.h>

/* 判断 c 数组的第一个元素是否 ch，并返回下一个指针 */
#define EXPECT_START_WITH(c, ch)\
    do {\
        assert(*c->json == (ch));\
        c->json++;\
    } while(0)

#define EXPECT_LEN_EQ(c, len)\
    do {\
        assert(sizeof(c) / sizeof(char) == len);\
    } while (0)

/**
 * 函数间传递参数：减少解析函数之间传递多个参数
 */
typedef struct {
    const char *json;
} lept_context;


const char true_arr[] = "rue";
const char false_arr[] = "alse";
const char null_arr[] = "ull";

/* 去除有效文本前面所有的空白字符 */
static void lept_parse_whitespace(lept_context *c) {
    const char *p = c->json;
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r')
        p++;
    c->json = p;
}

static int lept_parse_null(lept_context *c, lept_value *v, char* judge_arr) {
    EXPECT_START_WITH(c, 'n');
    char *p = judge_arr;
    while (*p != '\0') {
        if (c->json[0] != *p) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
        p++;
    }
    v->type = LEPT_NULL;
    return LEPT_PARSE_OK;
}

static int lept_parse_true(lept_context *c, lept_value *v, char* judge_arr) {
    EXPECT_START_WITH(c, 't');
    char *p = judge_arr;
    while (*p != '\0') {
        if (c->json[0] != *p) {
            return LEPT_PARSE_INVALID_VALUE;
        }
        c->json++;
        p++;
    }
    v->type = LEPT_TRUE;
    return LEPT_PARSE_OK;
}

static int lept_parse_false(lept_context *c, lept_value *v, char* judge_arr) {
    EXPECT_START_WITH(c, 'f');
    if (c->json[0] != 'a' || c->json[1] != 'l' || c->json[2] != 's' || c->json[3] != 'e') {
        return LEPT_PARSE_INVALID_VALUE;
    }

    /* 解析完成要把指针移动到末尾，用作下一步判断字符数组是否已结束 */
    c->json += 4;
    v->type = LEPT_FALSE;
    return LEPT_PARSE_OK;
}

static int lept_parse_value(lept_context *c, lept_value *v) {

    /* 只判断首字符 */
    switch (*c->json) {
        case 'n':
            return lept_parse_null(c, v, "ull");
        case 't':
            return lept_parse_true(c, v, "rue");
        case 'f':
            return lept_parse_false(c, v, "alse");
        case '\0':
            return LEPT_PARSE_EXPECT_VALUE;
        default:
            return LEPT_PARSE_INVALID_VALUE;
    }
}

int lept_parse(lept_value *v, const char *json) {
    lept_context c;
    int ret;
    assert(v != NULL);
    c.json = json;
    v->type = LEPT_NULL;
    lept_parse_whitespace(&c);
    ret = lept_parse_value(&c, v);

    /* JSON 文本应该有 3 部分：JSON-text = ws value ws；
     * 以下判断第三部分，即解析空白，然后检查 JSON 文本是否完结 */
    if (ret == LEPT_PARSE_OK) {
        lept_parse_whitespace(&c);
        if (*(c.json) != '\0') {
            ret = LEPT_PARSE_ROOT_NOT_SINGULAR;
        }
    }
    return ret;
}

lept_type lept_get_type(const lept_value *v) {
    assert(v != NULL);
    return v->type;
}
