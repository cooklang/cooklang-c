%{
#include "cook_syntax.c"
%}

%option noyywrap

%%
.   { printf("Found <%s>\n",yytext); return(TOKEN);}
