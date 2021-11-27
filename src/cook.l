%{
#include "cook_syntax.c"
%}

%option noyywrap

SYMB_CHAR "$"|"="|"+"|"-"|"_"|"*"|"`"|"'"
PUNC_CHAR             "!"|"?"|","|"."|"/"|"&"|"("|")"|":"
NEW_LINE              "\n"
WHITE_SPACE           " "|"\t"
ALPHA_CHAR            [a-zA-Z]

ZERO                  "0"
NON_ZERO_DIGIT        [1-9]

NUMBER                 {ZERO}|({NON_ZERO_DIGIT}({NON_ZERO_DIGIT}|{ZERO})*)

WORD ({ALPHA_CHAR}|{SYMB_CHAR})+
MULTIWORD ({WORD}{WHITE_SPACE})+
PHRASE ({WHITE_SPACE}*({PUNC_CHAR}|{WORD}|{MULTIWORD}){WHITE_SPACE}*)*

%%

{WORD} {printf("<%s>",yytext); return(TOKEN);}

{MULTIWORD} {printf("<%s>",yytext); return(TOKEN);}
{PHRASE} {printf("<%s>",yytext); return(TOKEN);}

{NUMBER} {printf("<%s>",yytext); return(TOKEN);}

{NEW_LINE} { printf("\n"); return(TOKEN);}

.   { printf("<%s>",yytext); return(TOKEN);}
