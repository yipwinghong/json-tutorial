#ifndef LEPTJSON_H__
#define LEPTJSON_H__
/* 宏的名字必须是唯一的，通常习惯以 _H__ 作为后缀：项目名称_目录_文件名称_H__ */

#include <stddef.h> /* size_t */

/* C 语言没有命名空间，一般会使用项目的简写作为标识符的前缀 */
/* 通常枚举值用全大写，类型及函数则用小写 */
typedef enum {
    LEPT_NULL,
    LEPT_FALSE,
    LEPT_TRUE,
    LEPT_NUMBER,
    LEPT_STRING,
    LEPT_ARRAY,
    LEPT_OBJECT
} lept_type;

/**
 * JSON 解析结果
 */
enum {
    LEPT_PARSE_OK = 0,              /* 解析成功 */
    LEPT_PARSE_EXPECT_VALUE,        /* JSON 只含有空白 */
    LEPT_PARSE_INVALID_VALUE,       /* 非合法的字面值 */
    LEPT_PARSE_ROOT_NOT_SINGULAR,   /* 一个值之后，在空白之后还有其他字符 */
    LEPT_PARSE_NUMBER_TOO_BIG,      /* 数值过大 */
    LEPT_PARSE_MISS_QUOTATION_MARK,
    LEPT_PARSE_INVALID_STRING_ESCAPE,
    LEPT_PARSE_INVALID_STRING_CHAR,
    LEPT_PARSE_INVALID_UNICODE_HEX,
    LEPT_PARSE_INVALID_UNICODE_SURROGATE
};


/**
 * JSON 的数据结构，以树形表示。即一个 JSON 值节点
 * lept_value 内使用了自身类型的指针，必须前向声明（forward declare）此类型：typedef struct {} lept_value; => struct lept_value
 */
typedef struct lept_value lept_value;


struct lept_value {
    union {
        struct {
            lept_value *e;
            size_t size;
        } a;
        struct {
            char *s;
            size_t len;
        } s;                    /* 字符串 */
        double n;               /* 数值（type == LEPT_NUMBER） */
    } u;                        /* 一个值不可能同时为数字和字符串，因此可以用 union 节省内存 */
    lept_type type;             /* 类型 */
};

/**
 * （调用访问函数前）对 JSON 对象类型初始化
 * do { ... } while(0) 把表达式转为语句，模仿无返回值的函数
 */
#define lept_init(v) do { (v)->type = LEPT_NULL; } while(0)

/**
 * 解析 JSON 字符串，用法：
 *      // 传入的根节点指针 v 是由使用方负责分配的
 *      lept_value v;
 *      const char json[] = ...;
 *      int ret = lept_parse(&v, json);
 *
 * @param v         JSON 值节点
 * @param json      JSON 字符串（常量数组）
 * @return          解析结果，见 {@link  }
 */
int lept_parse(lept_value *v, const char *json);

/**
 * 释放内存（用于字符串）
 *
 * @param v
 */
void lept_free(lept_value *v);

/**
 * 获取类型
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value *v);

/**
 * 设置空值
 */
#define lept_set_null(v) lept_free(v)

/**
 * 获取布尔值
 *
 * @param v
 * @return
 */
int lept_get_boolean(const lept_value *v);

/**
 * 设置布尔值
 *
 * @param v
 * @param b
 */
void lept_set_boolean(lept_value *v, int b);

/**
 * 获取数值
 *
 * @param v
 * @return
 */
double lept_get_number(const lept_value *v);

/**
 * 设置数值
 *
 * @param v
 * @param n
 */
void lept_set_number(lept_value *v, double n);

/**
 * 获取字符串值
 *
 * @param v
 * @return
 */
const char *lept_get_string(const lept_value *v);

/**
 * 获取字符串长度
 *
 * @param v
 * @return
 */
size_t lept_get_string_length(const lept_value *v);

/**
 * 设置字符串值
 *
 * @param v
 * @param s
 * @param len
 */
void lept_set_string(lept_value *v, const char *s, size_t len);

/**
 * 获取数组大小
 *
 * @param v
 * @return
 */
size_t lept_get_array_size(const lept_value *v);

/**
 * 获取数组元素
 *
 * @param v
 * @param index
 * @return
 */
lept_value* lept_get_array_element(const lept_value *v, size_t index);

#endif
