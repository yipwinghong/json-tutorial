#ifndef LEPTJSON_H__
#define LEPTJSON_H__
/* JSON 数据类型 */
typedef enum { 
    LEPT_NULL, 
    LEPT_FALSE, 
    LEPT_TRUE, 
    LEPT_NUMBER, 
    LEPT_STRING, 
    LEPT_ARRAY, 
    LEPT_OBJECT 
} lept_type;

/* JSON 解析结果 */
enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR
};

/* JSON 结构体 */
typedef struct {
    lept_type type;
} lept_value;

/*
 *
 *
 * 一般用法：
 *      lept_value v;
 *      const char json[] = ...;
 *      int ret = lept_parse(&v, json);
 *      */

/**
 * 解析 JSON，一般用法：
 *      lept_value v;
 *      const char json[] = ...;
 *      int ret = lept_parse(&v, json);
 * @param v     根节点指针
 * @param json  JSON 字符串
 * @return
 */
int lept_parse(lept_value* v, const char* json);

/**
 * 获取 JSON 类型
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value* v);

/* LEPTJSON_H__ */
#endif
