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
 *
LexRule r[]={
    [0]=symrule,
    [1]=lr_whitespace
};
 *  version : 0.1.1
 *  CHANGELOGS:
 *  0.1.1
 *    2026-05-10:
 *    	- ADDED LEKHADEF to add static inline if needed to  remove unused function warning
 *  0.1.2
 *    2026-05-12:
 *    	- fixed rule for skipping whitespace
 *    	- fixed rule skip extra iteration after finding the matching rule
 *    	- added dprintf for debug printing
 *    2026-05-14:
 *    	- fixed reallocation error while appending tokens
 *  
 */

#ifndef LEKHA_H
#define LEKHA_H
#include <stdint.h>

#ifndef LEKHADEF
    #define LEKHADEF
#endif
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

LEKHADEF int lex_init(Lexer* lex, char* buff, uint32_t len);

LEKHADEF int lex_analyse(Lexer* lex, LexRules rules);

LEKHADEF int lex_appendLToken(LTokens* toks, LToken tok);

LEKHADEF int lex_stringstart(const char* buf);
LEKHADEF int lex_stringhandle(Lexer* lex, uint32_t i);

LEKHADEF int lex_numberstart(const char* buf);
LEKHADEF int lex_numberhandle(Lexer* lex, uint32_t i);

LEKHADEF int lex_whitespacestart(const char* buf);
LEKHADEF int lex_whitespacehandle(Lexer* lex, uint32_t i);


typedef enum {
    LE_OK,
    LE_MISMATCH_CHAR,
    LE_TOKENIZE_ERROR,
    LE_NO_RULE_DEFINED,
    LE_TOK_LIST_ERROR
} LexError;

extern LexRule lr_whitespace;
extern LexRule lr_numbers;
extern LexRule lr_string;

#endif //LEX_H

#ifdef LEX_IMPLEMENTATION
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>

#ifdef lexdebug
    #define dprintf(...) printf(__VA_ARGS__)
#else
    #define dprintf(...)
#endif

LexRule lr_whitespace = {
    .startsWith = lex_whitespacestart,
    .handleLToken = lex_whitespacehandle
};

LexRule lr_numbers = {
    .startsWith = lex_numberstart,
    .handleLToken = lex_numberhandle
};

LexRule lr_string = {
    .startsWith = lex_stringstart,
    .handleLToken = lex_stringhandle
};

LEKHADEF int lex_init(Lexer* lex, char* buff, uint32_t len){
    lex->start = buff;
    lex->len = len;
    lex->tokens.capacity=8;
    lex->tokens.list=(LToken*)calloc(8,sizeof(LToken));
    return LE_OK;
}
LEKHADEF int lex_analyse(Lexer* lex, LexRules rules){
    while (lex->len > 0){
	dprintf("remaining: %.*s\n",lex->len,lex->start);
	uint32_t blen = lex->len;
	for(uint32_t i =0;i<rules.count;i++){
	    LexRule rule=rules.list[i];
	    if (rule.startsWith(lex->start) == LE_OK){
		LexError status = (LexError)rule.handleLToken(lex, i); 
		if (status == LE_OK)
		    break;
	    }
	}
	if (lex->len == blen){
	    return LE_NO_RULE_DEFINED;
	}
    }
    return LE_OK;
}

LEKHADEF int lex_appendLToken(LTokens* toks, LToken tok){
    if (toks->count == toks->capacity){
	toks->list = (LToken*)realloc(toks->list, toks->capacity*2* sizeof(tok));
	if (toks->list ==NULL){
	    return LE_TOK_LIST_ERROR;
	}
	toks->capacity *= 2;
    }
    toks->list[toks->count]=tok;
    toks->count++;
    return LE_OK;
}

LEKHADEF int lex_stringstart(const char* buf){
    int res =(buf[0] - '"'); 
    return (res == LE_OK)? res : LE_MISMATCH_CHAR;
}
LEKHADEF int lex_stringhandle(Lexer* lex, uint32_t i){
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

LEKHADEF int lex_numberstart(const char* buf){
    int isneg = buf[0] == '-';
    int ispos = isdigit(buf[0]);
    int res =isneg || ispos; 
    return (res == 1)? LE_OK : LE_MISMATCH_CHAR;
}
LEKHADEF int lex_numberhandle(Lexer* lex, uint32_t i){
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

LEKHADEF int lex_whitespacestart(const char* buf){
    int res = isspace(buf[0]);
    return (res != 0)? LE_OK : LE_MISMATCH_CHAR;
}
LEKHADEF int lex_whitespacehandle(Lexer* lex, uint32_t i){
    while(!lex_whitespacestart(lex->start) && lex->len > 0){
	lex->len--;
	lex->start++;
    }
    (void)i;
    return LE_OK;
}

LEKHADEF void debugLTokens(LTokens* toks){
    for (uint32_t i=0;i<toks->count;i++){
	LToken t = toks->list[i];
	printf("LToken ( lexme: %.*s , len:%d, type:%d)\n", t.size, t.lexme, t.size, t.type);
    }
}
#endif //LEX_IMPLEMENTATION
