#ifndef LEPTJSON_H__
#define LEPTJSON_H__

#include <stddef.h> /* size_t */

typedef enum {
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;

typedef struct {

    /*
     * 一个值不可能同时为数字和字符串，因此可以用 union 节省内存
     * s 和 n 分别表示当存储数据为字符串或数值时
     */
    union {
        struct {
            char *s;

            /* 字符串在设值时记录长度，避免访问时再次遍历 */
            size_t len;
        } s;                    /* string: null-terminated string, string length */
        double n;               /* number */
    } u;
    lept_type type;
} lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG,
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR
};

/**
 * （调用访问函数前）对 JSON 对象类型初始化
 * do { ... } while(0) 把表达式转为语句，模仿无返回值的函数
 */
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

int lept_parse(lept_value *v, const char *json);

void lept_free(lept_value *v);

lept_type lept_get_type(const lept_value *v);

#define lept_set_null(v) lept_free(v)

int lept_get_boolean(const lept_value *v);

void lept_set_boolean(lept_value *v, int b);

double lept_get_number(const lept_value *v);

void lept_set_number(lept_value *v, double n);

const char *lept_get_string(const lept_value *v);

size_t lept_get_string_length(const lept_value *v);

void lept_set_string(lept_value *v, const char *s, size_t len);

#endif /* LEPTJSON_H__ */
