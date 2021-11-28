%{
#include "cook_syntax.c"
#define yyterminate() return END_OF_FILE

%}

%option noyywrap

SYMB_CHAR "$"|"="|"+"|"_"|"*"|"`"|"'"|"-"
PUNC_CHAR             "!"|"?"|","|"."|"&"|"("|")"|":"|"/"
NEW_LINE              "\n"
WHITE_SPACE           " "|"\t"
ALPHA_CHAR            [a-zA-Z]

ZERO                  "0"
NON_ZERO_DIGIT        [1-9]

NUMBER                 {ZERO}|({NON_ZERO_DIGIT}({NON_ZERO_DIGIT}|{ZERO})*)

COMMENT_LINE "--".*""
COMMENT "[-".*"-]"
WORD {ALPHA_CHAR}({ALPHA_CHAR}|{SYMB_CHAR})*

MULTIWORD_WITH_LBRACE ({WORD}{WHITE_SPACE}+)*{WORD}"{"

 // MULTIWORD ({WORD}{WHITE_SPACE}*)+
 /* PHRASE ({WHITE_SPACE}*({PUNC_CHAR}|{WORD}|{MULTIWORD}){WHITE_SPACE}*)* */

%%

{WORD} {/* printf("WORD <%s>\n",yytext) ;*/ yylval.string = strdup(yytext); return(WORD);}

 /* {MULTIWORD} {printf("<%s>",yytext); yylval.string = strdup(yytext); return(MULTIWORD);} */
 /* {PHRASE} {printf("<%s>",yytext); yylval.string = strdup(yytext); return(PHRASE);} */

{NUMBER} {/* printf("NUMBER <%s> ",yytext); */ return(NUMBER);}

{NEW_LINE} { /*printf("<Newline>\n");*/ return(NEW_LINE);}

"@" {return(AT);}
"{" {return(LBRACE);}
"}" {return(RBRACE);}
"%" {return(PERCENT);}
"~" {return(TILDE);}
"#" {return(HASH);}
":" {return(COLON);}
">>" {return(METADATA_BEGIN);}
{WHITE_SPACE} {return(WHITE_SPACE);}
{MULTIWORD_WITH_LBRACE} { int length = strlen(yytext); yylval.string = strndup(yytext,length-1); return(MULTIWORD_WITH_LBRACE);}
{COMMENT_LINE} {return(COMMENT_LINE);}
{COMMENT} {return(COMMENT);}
{PUNC_CHAR} { yylval.string = strdup(yytext); return(PUNCTUATION);}

.   { printf("UNKNOWN <%s> ",yytext); yyerror("Not known"); return(UNKNOWN);}
