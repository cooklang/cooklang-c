%option noyywrap yylineno

%{
  #include <stdlib.h>
  #include "Cooklang.tab.h"
  #include "../include/CooklangParser.h"
  static int nextColumn = 1;
  int column = 1;

  #define TRACKCLMN column = nextColumn; nextColumn += strlen(yytext)

  void showError();
%}


NEW_LINE            \x0a|\x0d|\xc2\x85|\xe2\x80\xa8|\xe2\x80\xa9
WS_1                \x20|\xc2\xa0|\xe1\x9a\x80|\xe2\x80\x80|\xe2\x80\x81|\xe2\x80\x82|\xe2\x80\x83|\xe2\x80\x84|\xe2\x80\x85|\xe2\x80\x86|\xe2\x80\x87|\xe2\x80\x88|\xe2\x80\x89|\xe2\x80\x8a|\xe2\x80\xaf|\xe2\x81\x9f|\xe3\x80\x80|\x09
WHITE_SPACE         {WS_1}+
COOKLANG_CHAR       ">"|"~"|"@"|"#"|"{"|"}"|"%"


ALPHA_CHAR            [a-zA-Z]|":"
SYMB_CHAR             "$"|"="|"+"|"_"|"*"|"`"
PUNC_CHAR             "!"|"?"|","|"."|"/"|"&"|"("|")"|":"

ZERO                  "0"
NON_ZERO_DIGIT        [1-9]

DIGIT                 ({ZERO}|{NON_ZERO_DIGIT}){WHITE_SPACE}*
INTEGER               ({ZERO}|({NON_ZERO_DIGIT}{DIGIT}*)){WHITE_SPACE}*
DECIMAL               ({INTEGER}"."{INTEGER}){WHITE_SPACE}*
FRACTIONAL            ({INTEGER}{WHITE_SPACE}*"/"{WHITE_SPACE}*{INTEGER}){WHITE_SPACE}*


WORD                  {WHITE_SPACE}*({ALPHA_CHAR}|{DIGIT}|{SYMB_CHAR})+{PUNC_CHAR}*


HWORD                 "#"{WORD}
ATWORD                "@"{WORD}

MULTIWORD             {WHITE_SPACE}*{WORD}{2,}{WHITE_SPACE}*

UNIT                  "%"{WHITE_SPACE}*({WORD}|{MULTIWORD}*|{PUNC_CHAR}*)?

METADATA              ">"">".*

COMMENT               "-""-".*


%%


"{"                   { TRACKCLMN;
                        return LCURL; }
"}"                   { TRACKCLMN;
                        return RCURL; }
"~"                   { TRACKCLMN;
                        return TILDE; }
"--"                  { TRACKCLMN;
                        return COMMENT; }

{ATWORD}              { yytext++;
                        yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return ATWORD; }

{HWORD}               { yytext++;
                        yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return HWORD; }


{UNIT}                { yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return UNIT; }

{DIGIT}               { yylval.number = strtod(yytext, NULL); 
                        TRACKCLMN;
                        return NUMBER; }
                        
{INTEGER}             { yylval.number = strtod(yytext, NULL); 
                        TRACKCLMN;
                        return NUMBER; }

{DECIMAL}             { yylval.number = strtod(yytext, NULL); 
                        TRACKCLMN;
                        return NUMBER; }

{FRACTIONAL}          { char * tok = strtok(yytext, "/");

                        double first = strtod(tok, NULL);

                        tok = strtok(NULL, "/");

                        double second = strtod(tok, NULL);

                        double final = first/second;

                        yylval.number = final;

                        TRACKCLMN;
                        return NUMBER; }

{WORD}                { yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return WORD; }

{MULTIWORD}           { yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return MULTIWORD; }



{PUNC_CHAR}           { yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return PUNC_CHAR; }

{NEW_LINE}            { TRACKCLMN;
                        nextColumn = 1;
                        return NL; }

{METADATA}            { yylval.string = strdup(yytext);
                        return METADATA;
                        TRACKCLMN; }

{COMMENT}

{WHITE_SPACE}         { yylval.string = strdup(yytext);
                        TRACKCLMN;
                        return WHTS; }


%%



#ifdef LIB

char * testFile( char * fileName ){
  // open the input file
  FILE * file;

  if( fileName != NULL ){
    file = fopen(fileName, "r");
    if( file != NULL ){
      yyin = file;
    } else {
      return "NULL";
    }
  } else {
    return "NULL";
  }

  // open the output file

  FILE * outputFile = fopen("test_output.txt", "w");

  if(outputFile != NULL ){
    // setup the recipe
    Recipe * finalRecipe = createRecipe();
    // create the first step and insert into back of list
    Step * currentStep = createStep();
    insertBack(finalRecipe->stepList, currentStep);

    // run the parser on input file
    yyparse(finalRecipe);

    // get steps
    char * stepListString = toString(finalRecipe->stepList);

    // get metadata
    char * metaDataString = toString(finalRecipe->metaData);

    char * returnString = malloc( sizeof(char) * ( strlen(metaDataString) + strlen(stepListString) + 20) );

    sprintf(returnString, "%s%s", stepListString, metaDataString);

    free(metaDataString);
    free(stepListString);


    // free everything
    freeList(finalRecipe->metaData);
    freeList(finalRecipe->stepList);
    free(finalRecipe);

    return returnString;
  } else {
    return "NULL";
  }

  fclose(file);
  fclose(outputFile);

  return "NULL";
}


int runFile( char * fileName ){
  FILE * file;

  if( fileName != NULL ){
    file = fopen(fileName, "r");
    if( file != NULL ){
      yyin = file;
    } else {
      return 1;
    }
  } else {
    return 1;
  }

  // setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step
  Step * currentStep = createStep();

  insertBack(finalRecipe->stepList, currentStep);

  yyparse(finalRecipe);

  char * metaDataString = toString(finalRecipe->metaData);

  printf("%s\n", metaDataString);

  free(metaDataString);


  // printf out the step steps
  char * stepListString = toString(finalRecipe->stepList);

  printf("%s\n", stepListString);

  free(stepListString);


  freeList(finalRecipe->metaData);
  freeList(finalRecipe->stepList);
  free(finalRecipe);

  fclose(file);

  return 0;
}




#endif


#ifndef LIB
int main( int argc, char ** argv ){
  // yydebug = 1;
  ++argv;
  --argc;

  FILE * file = NULL;

  if( argc > 0 ){
    file = fopen(argv[0], "r");
    yyin = file;
  } else {
    yyin  = stdin;
  }

  // setup the recipe
  Recipe * finalRecipe = createRecipe();

  // create the first step
  Step * currentStep = createStep();

  insertBack(finalRecipe->stepList, currentStep);

  yyparse(finalRecipe);

  char * metaDataString = toString(finalRecipe->metaData);

  printf("%s\n", metaDataString);

  free(metaDataString);


  // printf out the step steps
  char * stepListString = toString(finalRecipe->stepList);

  printf("%s\n", stepListString);

  free(stepListString);


  freeList(finalRecipe->metaData);
  freeList(finalRecipe->stepList);
  free(finalRecipe);

  if( file != NULL ){
    fclose(file);
  }


  return 0;
}




#endif



#ifdef TEST
// main for testing
int main( int argc, char ** argv ){
  // yydebug = 1;
  ++argv;
  --argc;

  FILE * file = NULL;

  if( argc > 0 ){
    file = fopen(argv[0], "r");
    yyin = file;
  } else {
    yyin  = stdin;
  }


  char inputString[500] = ">> hey: there\n>> hello: again\n>> hey: there\n>> hey: there\n@chilli{ 3 \nmall bowl, combine the @avocado{1/2}\nhello\n";

  Recipe * finalRecipe = parseMultipleRecipeStrings(inputString);

  printf("\n\nParsing complete\n");

  char * metaDataString = toString(finalRecipe->metaData);
  printf("%s\n", metaDataString);
  free(metaDataString);

  // printf out the step steps
  char * stepListString = toString(finalRecipe->stepList);
  printf("%s\n", stepListString);
  free(stepListString);

  // free stuff
  freeList(finalRecipe->metaData);
  freeList(finalRecipe->stepList);
  free(finalRecipe);

  if( file != NULL ){
    fclose(file);
  }


  return 0;
}



#endif