

#ifndef SYRJSON_H_
#define SYRJSON_H_

#include<stddef.h> // size_t

//定义json中的数据类型  分别为 NULL，布尔false，布尔true，数字，字符串，数组，对象

typedef enum{	SYR_NULL, SYR_FALSE, SYR_TRUE, SYR_NUMBER, SYR_STRING,
			SYR_ARRAY, SYR_OBJECT} syr_type;


typedef struct{
	union{
		struct {
			char* s; size_t len;
		}s;

		double n;
	}u;

	syr_type type;
}syr_value;



//定义状态码
enum{
	SYR_PARSE_OK=0,
	SYR_PARSE_EXPECT_VALUE,
	SYR_PARSE_INVALID_VALUE,
	SYR_PARSE_ROOT_NOT_SINGULAR,
	SYR_PARSE_NUMBER_TOO_BIG,
	SYR_PARSE_MISS_QUOTATION_MARK,
	SYR_PARSE_INVALID_STRING_ESCAPE,
	SYR_PARSE_INVALID_STRING_CHAR,
	SYR_PARSE_INVALID_UNICODE_HEX,
	SYR_PARSE_INVALID_UNICODE_SURROGATE
};

#define syr_init(v)  do{(v)->type=SYR_NULL;}while(0)

#define syr_set_null(v) syr_free(v)

int syr_parse(syr_value* v,const char* json);

void syr_free(syr_value* v);

syr_type syr_get_type(const syr_value* v);

double syr_get_number(const syr_value* v);
void syr_set_number(syr_value* v,double n);

int syr_get_boolean(const syr_value* v);
void syr_set_boolean(syr_value* v,int b);

const char* syr_get_string(const syr_value* v);
size_t syr_get_string_length(const syr_value* v);
void syr_set_string(syr_value* v, const char* s,size_t len);

#endif /* SYRJSON_H_ */
