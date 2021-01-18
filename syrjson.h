/*
 * syrjson.h
 *
 *  Created on: 2021年1月17日
 *      Author: yxdshy
 */

#ifndef SYRJSON_H_
#define SYRJSON_H_

//定义json中的数据类型  分别为 NULL，布尔false，布尔true，数字，字符串，数组，对象

typedef enum{	SYR_NULL, SYR_FALSE, SYR_TRUE, SYR_NUMBER, SYR_STRING,
			SYR_ARRAY, SYR_OBJECT} syr_type;


typedef struct{
	double n;
	syr_type type;
}syr_value;



//定义状态码
enum{
	SYR_PARSE_OK=0,
	SYR_PARSE_EXPECT_VALUE,
	SYR_PARSE_INVALID_VALUE,
	SYR_PARSE_ROOT_NOT_SINGULAR,
	SYR_PARSE_NUMBER_TOO_BIG
};


int syr_parse(syr_value* v,const char* json);

syr_type syr_get_type(const syr_value* v);

double syr_get_number(const syr_value* v);

#endif /* SYRJSON_H_ */
