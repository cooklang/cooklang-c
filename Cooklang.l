
%{
  #include "Cooklang.tab.h"
  #include <stdlib.h>
  void showError();

%}


SYMB_CHAR             "$"|"="|"+"|"-"|"_"|"*"|"`"
PUNC_CHAR             "!"|"?"|","|"."|"/"|"&"|"("|")"|":"
NEW_LINE              "\n"
WHITE_SPACE           " "|"\t"
ALPHA_CHAR            [a-zA-Z]
COOKLANG_CHAR         ">"|"|"|"~"|"@"|"#"|":"|"{"|"}"|"%"



ZERO                  "0" 
NON_ZERO_DIGIT        [0-9]


DIGIT                 ({ZERO}|{NON_ZERO_DIGIT}){WHITE_SPACE}*
INTEGER               ({ZERO}|({NON_ZERO_DIGIT}{DIGIT}*)){WHITE_SPACE}*
DECIMAL               {INTEGER}"."{INTEGER}{WHITE_SPACE}*
FRACTIONAL            {INTEGER}{WHITE_SPACE}*"/"{WHITE_SPACE}*{INTEGER}{WHITE_SPACE}*


WORD                  ({ALPHA_CHAR}|{DIGIT}|{SYMB_CHAR})+{WHITE_SPACE}*

HWORD                 "#"{WHITE_SPACE}*{WORD}
ATWORD                "@"{WHITE_SPACE}*{WORD}

MULTIWORD             {WORD}{2,}


UNIT                  "%"{WHITE_SPACE}*({MULTIWORD}*|{PUNC_CHAR}*)?



%%

[ \t]

"{"                   {return LCURL;}
"}"                   {return RCURL;}
"~"                   {return TILDE;}

{ATWORD}              { yylval.string = malloc(strlen(yytext) + 1);
                        yytext++;
                        sprintf(yylval.string, "%s", yytext);
                        return ATWORD;
}

{HWORD}               { yylval.string = malloc(strlen(yytext) + 1);
                        yytext++;
                        sprintf(yylval.string, "%s", yytext);
                        return HWORD;
                      }

                      
{UNIT}                { yylval.string = malloc(strlen(yytext) + 5);
                        yytext++;
                        sprintf(yylval.string, "%s", yytext);
                        return UNIT;}

{DIGIT}               { yylval.number = strtod(yytext, NULL); return NUMBER;}
{INTEGER}             { yylval.number = strtod(yytext, NULL); return NUMBER;}
{DECIMAL}             { yylval.number = strtod(yytext, NULL); return NUMBER;}
{FRACTIONAL}          { char * tok = strtok(yytext, "/");

                        double first = strtod(tok, NULL);

                        tok = strtok(NULL, "/");

                        double second = strtod(tok, NULL);

                        double final = first/second;

                        yylval.number = final;

                        return NUMBER;}

{WORD}                { yylval.string = malloc(strlen(yytext) + 5);
                        sprintf(yylval.string, "%s", yytext);
                        return WORD;
                      }
{MULTIWORD}           { yylval.string = malloc(strlen(yytext) + 5);
                        sprintf(yylval.string, "%s", yytext);
                        return MULTIWORD;
                      }



{PUNC_CHAR}           { yylval.character = yytext[0];
                        return PUNC_CHAR; }

{NEW_LINE}            {return NL;}




%%

int main( int argc, char ** argv ){
  ++argv;
  --argc;

  if( argc > 0 ){
    yyin = fopen(argv[0], "r");
  } else {
    yyin  = stdin;
  }

  yyparse();

  yylex();
  
  printf("\n");
  
  return 0;
}