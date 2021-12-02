
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
NUMBER                {DIGIT}|{INTEGER}|{DECIMAL}|{FRACTIONAL}

WORD                  ({ALPHA_CHAR}|{DIGIT}|{SYMB_CHAR})+{WHITE_SPACE}*

HWORD                 "#"{WORD}
ATWORD                "@"{WORD}

MULTIWORD             {WORD}{2,}


UNIT                  "%"{WHITE_SPACE}*({MULTIWORD}*|{PUNC_CHAR}*)?

METADATA              ">"">"{WHITE_SPACE}*({WORD}|{MULTIWORD})":"{WHITE_SPACE}*({PUNC_CHAR}|{WORD}|{MULTIWORD}|{NUMBER})*


%%

[ \t]

"{"                   {return LCURL;}
"}"                   {return RCURL;}
"~"                   {return TILDE;}

{ATWORD}              { yytext++;
                        yylval.string = strdup(yytext);
                        return ATWORD;
}

{HWORD}               { yytext++;
                        yylval.string = strdup(yytext);
                        return HWORD;
                      }

                      
{UNIT}                { yylval.string = strdup(yytext);
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

{WORD}                { yylval.string = strdup(yytext);
                        return WORD;
                      }
{MULTIWORD}           { yylval.string = strdup(yytext);
                        return MULTIWORD;
                      }



{PUNC_CHAR}           { char c = yytext[0];
                        yylval.character = c;
                        return PUNC_CHAR; }

{NEW_LINE}            {return NL;}

{METADATA}            { yytext++;
                        yytext++;
                        yylval.string = strdup(yytext);
                        return METADATA;
                      }


%%

int main( int argc, char ** argv ){
  ++argv;
  --argc;

  if( argc > 0 ){
    yyin = fopen(argv[0], "r");
  } else {
    yyin  = stdin;
  }

  // setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step
  Step * currentStep = createStep();

  insertBack(finalRecipe->stepList, currentStep);

  printf("asdf\n");

  yyparse(finalRecipe);

  printf("asdf2\n");

  

  printf("End of parsing\n");

  // printf out the step steps 


  char * stepListString = toString(finalRecipe->stepList);

  printf("%s\n", stepListString);

  return 0;
}