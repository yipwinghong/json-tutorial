#ifndef LEPTJSON_H__
#define LEPTJSON_H__

typedef enum {
    LEPT_NULL, LEPT_FALSE, LEPT_TRUE, LEPT_NUMBER, LEPT_STRING, LEPT_ARRAY, LEPT_OBJECT
} lept_type;

/**
 * JSON 中没有限制数字的范围和精度，此处用 double 存储数字
 */
typedef struct {
    double n;

    /**
     * 仅当 type == LEPT_NUMBER 时，n 才表示 JSON 数字的数值
     */
    lept_type type;
} lept_value;

enum {
    LEPT_PARSE_OK = 0,
    LEPT_PARSE_EXPECT_VALUE,
    LEPT_PARSE_INVALID_VALUE,
    LEPT_PARSE_ROOT_NOT_SINGULAR,
    LEPT_PARSE_NUMBER_TOO_BIG
};

/**
 * 解析 JSON
 *
 * @param v
 * @param json
 * @return
 */
int lept_parse(lept_value *v, const char *json);

/**
 * 获取 JSON 类型
 *
 * @param v
 * @return
 */
lept_type lept_get_type(const lept_value *v);

/**
 * 获取 JSON 数值
 *
 * @param v
 * @return
 */
double lept_get_number(const lept_value *v);

#endif /* LEPTJSON_H__ */
