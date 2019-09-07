#ifndef LEPTJSON_H__
#define LEPTJSON_H__
/* 宏的名字必须是唯一的，通常习惯以 _H__ 作为后缀：项目名称_目录_文件名称_H__ */

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
    /* 解析成功 */
    LEPT_PARSE_OK = 0,

    /* JSON 只含有空白 */
    LEPT_PARSE_EXPECT_VALUE,

    /* 非合法的字面值 */
    LEPT_PARSE_INVALID_VALUE,

    /* 一个值之后，在空白之后还有其他字符 */
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

/**
 * JSON 的数据结构，以树形表示。即一个 JSON 值节点
 */
typedef struct {
    lept_type type;
} lept_value;

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
 * 获取 JSON 值
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value *v);

#endif
