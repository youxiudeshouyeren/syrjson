#include "syrjson.h"
#include <assert.h>   //assert()
#include <stdlib.h>   //NULL strtod()
#include<stdio.h>
#include<errno.h>  //errno,ERANGE
#include<math.h>   //HUGE_VAL

//检测字符是否为所需字符的宏定义
#define EXPECT(c,ch)  do{ assert(*c->json==(ch));c->json++;}while(0)

//检测是否为0-9
#define ISDIGIT(ch)  ((ch)>='0' &&(ch)<='9')

#define ISDIGIT1TO9(ch) ((ch)>='1' && ((ch)<='9'))

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

#if 0
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
#endif

//解析数字
static int syr_parse_number(syr_context* c,syr_value* v){

	const char* p=c->json;

	if(*p=='-')p++;

	if(*p=='0')
		p++;
	else{
		if(!ISDIGIT1TO9(*p))
			return SYR_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}

	if(*p=='.'){
		p++;
		if(!ISDIGIT(*p))
			return SYR_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}

	if(*p=='e'||*p=='E'){
		p++;
		if(*p=='+'||*p=='-')
			p++;
		if(!ISDIGIT(*p))
			return SYR_PARSE_INVALID_VALUE;
		for(p++;ISDIGIT(*p);p++);
	}



	errno=0;
	v->n=strtod(c->json,NULL);
	//待查
	if(errno==ERANGE &&((v->n==HUGE_VAL)||v->n==-HUGE_VAL))
		return SYR_PARSE_NUMBER_TOO_BIG;


	v->type=SYR_NUMBER;
	c->json=p;
	return SYR_PARSE_OK;


}



static int syr_parse_literal(syr_context* c,syr_value* v,const char* literal,syr_type type){

	size_t i;// i为literal长度减1
	EXPECT(c,literal[0]);  //使用EXPECT 后，c->json指针向后了一个 因此需要比较的literal向后一位
	for(i=0;literal[i+1];i++){
		if(c->json[i]!=literal[i+1])
			return SYR_PARSE_INVALID_VALUE;
	}

	c->json+=i;
	v->type=type;

	return SYR_PARSE_OK;
}
//
static int syr_parse_value(syr_context *c, syr_value* v){
	switch(*c->json){
	case 'n': return syr_parse_literal(c, v, "null", SYR_NULL);
	case 't':return  syr_parse_literal(c, v, "true", SYR_TRUE);//检测true
	case 'f':return syr_parse_literal(c, v,"false",SYR_FALSE);//检测false
    default: return syr_parse_number(c,v);
	case '\0':return SYR_PARSE_EXPECT_VALUE;

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

double syr_get_number(const syr_value* v){
	assert(v!=NULL && v->type==SYR_NUMBER);

	return v->n;
}

