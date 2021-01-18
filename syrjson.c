#include "syrjson.h"
#include <assert.h>   //assert()
#include <stdlib.h>   //NULL
#include<stdio.h>

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
	if(c->json[0]!='u'||c->json[1]!='l'||c->json[2]!='l'){
		return SYR_PARSE_INVALID_VALUE;
	}

	c->json+=3;
	v->type=SYR_NULL;

	return SYR_PARSE_OK;
}


//解析true
static int syr_parse_true(syr_context* c,syr_value* v){
	EXPECT(c,'t');

	//true检测
	if(c->json[0]!='r'||c->json[1]!='u'||c->json[2]!='e'){
		return SYR_PARSE_INVALID_VALUE;
	}

	c->json+=3;
	v->type=SYR_TRUE;

	return SYR_PARSE_OK;
}


//解析false
static int syr_parse_false(syr_context* c,syr_value* v){
	EXPECT(c,'f');

	//false检测
	if(c->json[0]!='a'||c->json[1]!='l'||c->json[2]!='s'||c->json[3]!='e'){
		return SYR_PARSE_INVALID_VALUE;
	}

	c->json+=4;
	v->type=SYR_FALSE;

	return SYR_PARSE_OK;
}

//
static int syr_parse_value(syr_context *c, syr_value* v){
	switch(*c->json){
	case 'n': return syr_parse_null(c,v);
	case 't':return  syr_parse_true(c,v);//检测true
	case 'f':return syr_parse_false(c, v);//检测false

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

	//检测ws后还有字符
	int status_code=syr_parse_value(&c, v);
	syr_parse_whitespace(&c);
	if(c.json[0]!='\0'){
		status_code=SYR_PARSE_ROOT_NOT_SINGULAR; //空白字符后面还有值

	//	printf("%s   ",c.json);
	//	printf("%c\n",c.json[0]);
	}
	return status_code;
}


syr_type syr_get_type(const syr_value* v){
	assert(v!=NULL);
	return v->type;
}

