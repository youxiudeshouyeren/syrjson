#include "syrjson.h"
#include <assert.h>   //assert()
#include <stdlib.h>   //NULL strtod() malloc realloc free
#include<stdio.h>
#include<errno.h>  //errno,ERANGE
#include<math.h>   //HUGE_VAL
#include<string.h>

#ifndef SYR_PARSE_STACK_INIT_SIZE
#define SYR_PARSE_STACK_INIT_SIZE 256
#endif



//检测字符是否为所需字符的宏定义
#define EXPECT(c,ch)  do{ assert(*c->json==(ch));c->json++;}while(0)

//检测是否为0-9
#define ISDIGIT(ch)  ((ch)>='0' &&(ch)<='9')

#define ISDIGIT1TO9(ch) ((ch)>='1' && ((ch)<='9'))

//压栈操作
#define PUTC(c,ch)  do{*(char*)syr_context_push(c,sizeof(char))=(ch);}while(0)

//json数据
typedef struct{
	const char* json;
	char* stack;
	size_t size,top;
}syr_context;



static void* syr_context_push(syr_context* c,size_t size){
	void* ret;
	assert(size>0);
	if(c->top+size>=c->size){
		if(c->size==0)
		{
			c->size=SYR_PARSE_STACK_INIT_SIZE;
		}
		while(c->top+size>=c->size){
			c->size+=c->size>>1;// 扩容1.5倍
		}
		c->stack=(char*)realloc(c->stack,c->size);  //重新分配内存
	}
	ret=c->stack+c->top;  //栈顶指针位置 用于宏定义PUTC 中赋值
	c->top+=size;
	return ret;
}

static void* syr_context_pop(syr_context* c,size_t size){
	assert(c->top>=size);
	return c->stack+(c->top-=size);
}

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
	v->u.n=strtod(c->json,NULL);
	//待查
	if(errno==ERANGE &&((v->u.n==HUGE_VAL)||v->u.n==-HUGE_VAL))
		return SYR_PARSE_NUMBER_TOO_BIG;


	v->type=SYR_NUMBER;
	c->json=p;
	return SYR_PARSE_OK;


}

static const char* syr_parse_hex4(const char* p,unsigned* u){
	int i;
	*u=0;
	for(i=0;i<4;i++){
		char ch=*p++;
		*u<<=4;
		if	(ch>='0' && ch<='9')
			*u|=ch-'0';
		else if(ch>='A'&&ch<='F')
			*u|=ch-('A'-10);
		else if(ch>='a'&&ch<='f')
			*u|=ch-('a'-10);
		else return NULL;

	}
	return p;
}

static void syr_encode_utf8(syr_context* c,unsigned u){
	if(u<=0x7F)
		PUTC(c,u&0xFF);
	else if(u<=0x7FF){
		PUTC(c,0xC0|((u>>6)&0xFF));
		PUTC(c,0xb0|(u&0x3F));
	}
	else if(u<=0xFFFF){
		PUTC(c,0xE0|((u>>12)&0xFF));
		PUTC(c,0x80|((u>>6)&0x3F));
		PUTC(c,0x80|(u&0x3F));
	}
	else{
		assert(u<=0x10FFFF);
		PUTC(c,0xF0|((u>>18)&0xFF));
		PUTC(c,0x80|((u>>12)&0x3F));
		PUTC(c,0x80|((u>>6)&0x3F));
		PUTC(c,0x80|(u&0x3F));
	}
}

#define STRING_ERROR(ret) do { c->top = head; return ret; } while(0)

static int syr_parse_string(syr_context* c,syr_value* v){

	size_t head=c->top,len;
	const char* p;
	unsigned u,u2;
	EXPECT(c,'\"');
	p=c->json;
	for(;;){
		char ch=*p++;
		switch(ch){

		case'\"': //结束的双引号标志
			len=c->top - head;
			syr_set_string(v,(const char*)syr_context_pop(c,len),len);
			c->json=p;
			return SYR_PARSE_OK;

		case '\\':
		{
			switch(*p++){
			case '\"':PUTC(c,'\"');break;
			case '\\':PUTC(c,'\\');break;
			case '/': PUTC(c,'/');break;
			case 'b': PUTC(c,'\b');break;
			case 'f': PUTC(c,'\f');break;
			case 'n': PUTC(c,'\n');break;
			case 'r': PUTC(c,'\r');break;
			case 't': PUTC(c,'\t');break;
			case 'u':
				if(!(p=syr_parse_hex4(p,&u)))
					STRING_ERROR(SYR_PARSE_INVALID_UNICODE_HEX);
				if(u>=0xD800&&u<=0xDBFF){

					//代理对
					if(*p++!='\\')
						STRING_ERROR(SYR_PARSE_INVALID_UNICODE_SURROGATE);
					if(*p++!='u')
						STRING_ERROR(SYR_PARSE_INVALID_UNICODE_SURROGATE);
					if(!(p=syr_parse_hex4(p,&u2)))
						STRING_ERROR(SYR_PARSE_INVALID_UNICODE_HEX);
					if(u2<0xDC00||u2>0xDFFFF)
					{
						STRING_ERROR(SYR_PARSE_INVALID_UNICODE_SURROGATE);
					}
					u = (((u - 0xD800) << 10) | (u2 - 0xDC00)) + 0x10000;

				}

				syr_encode_utf8(c,u);
				break;
			default:
				c->top=head;
				return SYR_PARSE_INVALID_STRING_ESCAPE; //非法转义字符
			}
		}
             break;
		case '\0':
			c->top=head;
			return SYR_PARSE_MISS_QUOTATION_MARK; //缺少引号
		default:
			if((unsigned char)ch< 0x20){
				c->top=head;
				return SYR_PARSE_INVALID_STRING_CHAR; //非法字符
			}
			PUTC(c,ch);

		}
	}

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
    case '"':return syr_parse_string(c,v);
	case '\0':return SYR_PARSE_EXPECT_VALUE;

	}
}


int syr_parse(syr_value* v, const char* json){
	syr_context c;

	assert(v!=NULL);
	c.json=json;
	c.stack=NULL;
	c.size=c.top=0;
	syr_init(v);
	syr_parse_whitespace(&c);

	//检测ws后还有字符
	int status_code=syr_parse_value(&c, v);

	if(status_code==SYR_PARSE_OK){
		syr_parse_whitespace(&c);
		if(*c.json!='\0')
		status_code=SYR_PARSE_ROOT_NOT_SINGULAR; //空白字符后面还有值


	}
	assert(c.top==0);//解析结束确保栈中数据全部弹出
	free(c.stack);
	return status_code;
}

void syr_free(syr_value* v){
	assert(v!=NULL);
	if(v->type==SYR_STRING)
		free(v->u.s.s);

	v->type=SYR_NULL;
}
syr_type syr_get_type(const syr_value* v){
	assert(v!=NULL);
	return v->type;
}

void syr_set_boolean(syr_value* v,int b){
    syr_free(v);
    v->type=b?SYR_TRUE:SYR_FALSE;
}

int syr_get_boolean(const syr_value* v){
	assert(v!=NULL&&(v->type==SYR_TRUE || v->type==SYR_FALSE));
	return v->type==SYR_TRUE;
}

double syr_get_number(const syr_value* v){
	assert(v!=NULL && v->type==SYR_NUMBER);

	return v->u.n;
}

void syr_set_number(syr_value* v,double n){
	syr_free(v);

	v->type=SYR_NUMBER;

	v->u.n=n;

}

const char* syr_get_string(const syr_value* v){
	assert(v!=NULL&&v->type==SYR_STRING);
	return v->u.s.s;

}

size_t syr_get_string_length(const syr_value* v){
	printf("%s\n",v->u.s.s);
	assert(v!=NULL&&v->type==SYR_STRING);
	return v->u.s.len;
}

void syr_set_string(syr_value* v,const char* s,size_t len){
	assert(v!=NULL&&(s!=NULL ||len==0));
	syr_free(v);
	v->u.s.s=(char*)malloc(len+1);
	memcpy(v->u.s.s,s,len);
	v->u.s.s[len]='\0';
	v->u.s.len=len;
	v->type=SYR_STRING;

}

