%option yylineno

%{
#include "parser.tab.h"
#include <stdlib.h>
#include <string.h>

int curr_col = 1;

#define YY_USER_ACTION \
    yylloc.first_line = yylloc.last_line = yylineno; \
    yylloc.first_column = curr_col; \
    yylloc.last_column = curr_col + yyleng - 1; \
    curr_col += yyleng;
%}

%%

"if" 					{ return IF;}

"else" 				{ return ELSE; }

"while" 			{ return WHILE; }

"return" 			{ return RETURN; }

"break" 			{ return BREAK; }

"int" 				{
								yylval.type = TYPE_INT;
								return TYPE;
							}

"true" 				{ return TRUE; }

"false" 			{ return FALSE; }

"<=" 					{
								yylval.op = OP_RELOP_LEQ;
								return RELOP;
							}

"<" 					{
								yylval.op = OP_RELOP_LT;
								return RELOP;
							}

">" 					{
								yylval.op = OP_RELOP_GT;
								return RELOP;
							}

">=" 					{
								yylval.op = OP_RELOP_GEQ;
								return RELOP;
							}

"==" 					{
								yylval.op = OP_RELOP_EQ;
								return RELOP;
							}

"!=" 					{
								yylval.op = OP_RELOP_NEQ;
								return RELOP;
							}

[+-] 					{
								switch(yytext[0]) {
									case '+':
										yylval.op = OP_ADDOP_ADD;
										break;

									case '-':
										yylval.op = OP_ADDOP_SUB;
								}
								return ADDOP;
							}

[*/] 					{
								switch(yytext[0]) {
									case '*':
										yylval.op = OP_MULOP_MUL;
										break;

									case '/':
										yylval.op = OP_MULOP_DIV;
								}
								return MULOP;
							}

[a-zA-Z][a-zA-Z0-9_]* {
								yylval.id = strdup(yytext);
								return IDENTIFIER;
							}

-?([1-9][0-9]*|0) {
								yylval.num = atoi(yytext);
								return NUMBER;
							}

[ \t\r]+   		{ /* ignore */ }
\n 						{ curr_col = 1; }


"//".*        { /* SKIP COMMENTS */ }

. 						return yytext[0];

%%
