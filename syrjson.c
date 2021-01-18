#include "syrjson.h"
#include <assert.h>   //assert()
#include <stdlib.h>   //NULL

//检测字符是否为所需字符的宏定义
#define EXPECT(c,ch)  do{ assert(*c->json==(ch));c->json++;}while(0)


//json数据
typedef struct{
	const char* json;
}syr_context;


//检测空格 直到有效数据
static void syr_parse_whitespace(syr_context* c){
	const char* p=c->json;


	//检测到 空格 换行符 制表符 回车符  指针向后直到有效数据
	while(*p==' '||*p=='\n'||*p=='\t'||*p=='\r'){
		p++;
	}
	c->json=p;

}

//解析null
static int syr_parse_null(syr_context* c,syr_value* v ){
	EXPECT(c, 'n');

	//null检测
	if(c->json[0]!='u'||c->json[1]!='l'||c->json[2]=='l'){
		return SYR_PARSE_INVALID_VALUE;
	}

	c->json+=3;
	v->type=SYR_NULL;

	return SYR_PARSE_OK;
}


//
static int syr_parse_value(syr_context *c, syr_value* v){
	switch(*c->json){
	case 'n': return syr_parse_null(c,v);
	case '\0':return SYR_PARSE_EXPECT_VALUE;
	default: return SYR_PARSE_INVALID_VALUE;
	}
}


int syr_parse(syr_value* v, const char* json){
	syr_context c;
	assert(v!=NULL);
	c.json=json;
	v->type=SYR_NULL;
	syr_parse_whitespace(&c);
	return syr_parse_value(&c, v);
}


syr_type syr_get_type(const syr_value* v){
	assert(v!=NULL);
	return v->type;
}

