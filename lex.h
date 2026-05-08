/*
 * lex.h
 * generic lexer implementation
 *
 *   Copyright 2026 Adarsh Shaw <adarshwshaw@gmail.com>
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */

#ifndef LEKHA_H
#define LEKHA_H
#include <stdint.h>

typedef struct {
    char* lexme;
    uint32_t size;
    uint32_t type;
} LToken;
typedef struct {
    uint32_t count,capacity;
    LToken* list;
} LTokens;

typedef struct {
    char* start;
    uint32_t len;
    LTokens tokens;
} Lexer;

typedef struct {
    int (*startsWith)(const char*); // return 0 on true and 1 on false
    int (*handleLToken)(Lexer* lex, uint32_t i); // return 0 on success and non-zero on false
} LexRule;

typedef struct {
    uint32_t count, capacity;
    LexRule* list;
} LexRules;

static int lex_init(Lexer* lex, char* buff, uint32_t len);

static int lex_analyse(Lexer* lex, LexRules rules);

static int lex_appendLToken(LTokens* toks, LToken tok);

static int lex_stringstart(const char* buf);
static int lex_stringhandle(Lexer* lex, uint32_t i);

static int lex_numberstart(const char* buf);
static int lex_numberhandle(Lexer* lex, uint32_t i);

static int lex_whitespacestart(const char* buf);
static int lex_whitespacehandle(Lexer* lex, uint32_t i);


typedef enum {
    LE_OK,
    LE_MISMATCH_CHAR,
    LE_TOKENIZE_ERROR,
    LE_NO_RULE_DEFINED,
    LE_TOK_LIST_ERROR
} LexError;

static LexRule lr_whitespace = {
    .startsWith = lex_whitespacestart,
    .handleLToken = lex_whitespacehandle
};

static LexRule lr_numbers = {
    .startsWith = lex_numberstart,
    .handleLToken = lex_numberhandle
};

static LexRule lr_string = {
    .startsWith = lex_stringstart,
    .handleLToken = lex_stringhandle
};


#endif //LEX_H

#ifdef LEX_IMPLEMENTATION
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

static int lex_init(Lexer* lex, char* buff, uint32_t len){
    lex->start = buff;
    lex->len = len;
    lex->tokens.capacity=8;
    lex->tokens.list=(LToken*)calloc(8,sizeof(LToken));
    return LE_OK;
}
int lex_analyse(Lexer* lex, LexRules rules){
    while (lex->len > 0){
	printf("remaining: %.*s\n",lex->len,lex->start);
	uint32_t blen = lex->len;
	for(uint32_t i =0;i<rules.count;i++){
	    LexRule rule=rules.list[i];
	    if (rule.startsWith(lex->start) == LE_OK){
		LexError status = (LexError)rule.handleLToken(lex, i); 
	    }
	}
	if (lex->len == blen){
	    return LE_NO_RULE_DEFINED;
	}
    }
    return LE_OK;
}

int lex_appendLToken(LTokens* toks, LToken tok){
    if (toks->count == toks->capacity){
	toks->list = (LToken*)realloc(toks->list, toks->capacity*2* sizeof(tok));
	if (toks->list ==NULL){
	    return LE_TOK_LIST_ERROR;
	}
    }
    toks->list[toks->count]=tok;
    toks->count++;
    return LE_OK;
}

static int lex_stringstart(const char* buf){
    int res =(buf[0] - '"'); 
    return (res == LE_OK)? res : LE_MISMATCH_CHAR;
}
static int lex_stringhandle(Lexer* lex, uint32_t i){
    lex->len--;
    lex->start++;
    char* lexme = lex->start;
    uint32_t len=0;
    while (lex->start[0] != '"'){
	len++;
	lex->len--;
	lex->start++;
    }
    lex->len--;
    lex->start++;

    LToken t ={
	.type=i,
	.lexme=lexme,
	.size=len
    };
    return lex_appendLToken(&(lex->tokens), t);
}

static int lex_numberstart(const char* buf){
    int isneg = buf[0] == '-';
    int ispos = isdigit(buf[0]);
    int res =isneg || ispos; 
    return (res == 1)? LE_OK : LE_MISMATCH_CHAR;
}
static int lex_numberhandle(Lexer* lex, uint32_t i){
    int len=0;
    int dec_count=0;
    char* lexme=lex->start;
    if (lex->start[0]=='-'){
	len++;
	lex->len--;
	lex->start++;
    }
    while (lex->len > 0){
	if (lex->start[0] == '.' || isdigit(lex->start[0])){
	    if (lex->start[0] == '.' && dec_count!=0){
		return LE_MISMATCH_CHAR;
	    }else{
		len++;
		lex->len--;
		lex->start++;
	    }
	}else{
	    break;
	}
    }
    LToken t={
	.type=i,
	.lexme=lexme,
	.size=(uint32_t)len
    };

    return lex_appendLToken(&(lex->tokens), t);
}

static int lex_whitespacestart(const char* buf){
    int res = isblank(buf[0]);
    return (res == 1)? LE_OK : LE_MISMATCH_CHAR;
}
static int lex_whitespacehandle(Lexer* lex, uint32_t i){
    while(!lex_whitespacestart(lex->start) && lex->len > 0){
	lex->len--;
	lex->start++;
    }
    return LE_OK;
}

static void debugLTokens(LTokens* toks){
    for (uint32_t i=0;i<toks->count;i++){
	LToken t = toks->list[i];
	printf("LToken ( lexme: %.*s , len:%d, type:%d)\n", t.size, t.lexme, t.size, t.type);
    }
}
#endif //LEX_IMPLEMENTATION
