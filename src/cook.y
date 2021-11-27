%{
#include <stdio.h>

#define YYDEBUG 1
  int yylex(void);
  void yyerror(const char* msg);
  extern FILE * yyin;

%}

%token TOKEN
%start recipe
%%

recipe:         TOKEN
        |       recipe TOKEN
        ;

%%

void yyerror(const char* msg)
{
  printf("%s \n",msg);
}
