#include <stdio.h>
#define LEX_IMPLEMENTATION
#include "lex.h"
#include <string.h>


int main(void){
    LexRules rules = {
	.count=3,
	.capacity=3,
	.list = (LexRule[]){
	    lr_whitespace,
	    lr_numbers,
	    lr_string
	}
    };

    char *file = "    123 321.123 -123 \"meta\"";
    Lexer lex={0};
    lex_init(&lex,file,strlen(file));
    LexError err = lex_analyse(&lex, rules);
    printf("status: %d\n",err);
    debugLTokens(&lex.tokens);
    printf("hello\n");
    return 0;
}
