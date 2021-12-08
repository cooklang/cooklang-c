
%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/CooklangParser.h"

#define YYDEBUG 1

int yylex();

int yyerror ( Recipe * recipe, const char * s);

extern void yyrestart( FILE * input_file );



%}

%parse-param {Recipe * recipe}


%union{
  char * string;
  char character;
  double number;
}

%token WORD MULTIWORD UNIT NUMBER LCURL RCURL PUNC_CHAR NL TILDE HWORD ATWORD METADATA

%type <character> LCURL RCURL PUNC_CHAR NL
%type <string> WORD MULTIWORD UNIT HWORD ATWORD METADATA
%type <number> NUMBER

%type <string> text_item ingredient cookware timer amount cookware_amount
%type <string> input line step direction


%define parse.error verbose

%expect 20

%%

input:
  %empty
  | input line
  ;


line:
    NL      {}
  | step NL {
      // after a step has been finished by a new line, have to add the step to the steplist
      // and make a new step to accept directions
      Step * newStep = createStep();

      insertBack(recipe->stepList, newStep);
      free($1);
    }
  | METADATA NL {
      // add metadata to the recipe
      addMetaData(recipe, $1);
      free($1);
    }
  ;


step:
    direction {
        $$ = strdup($1);
        free($1);
      }
  | step direction {
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s %s", $1, $2);
      free($1);
      free($2);
    }
  ;


direction:
    text_item {
      $$ = strdup($1);
      addDirection(recipe, "TextItem", $1, NULL);
      free($1);
    }
  | timer             
  | cookware          
  | ingredient        
  | HWORD text_item   {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", $1, NULL);
      addDirection(recipe, "TextItem", $2, NULL);
      free($1);
      free($2);
    }
  | ATWORD text_item  {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Ingredient", $1, NULL);
      addDirection(recipe, "TextItem", $2, NULL);
      free($1);
      free($2);
    }
  ;



text_item:
    WORD
  | MULTIWORD
  | NUMBER  { 
      $$ = malloc(10);
      sprintf($$, "%.3f", $1);
    }

  | PUNC_CHAR { 
      $$ = malloc(5);
      sprintf($$, "%c", $1);
    }
    
  | text_item WORD  { 
      $$ = addTwoStrings($1, $2);
      free($1);
      free($2);
    }
    
  | text_item MULTIWORD  {
      $$ = addTwoStrings($1, $2);
      free($1);
      free($2);
    }

  | text_item NUMBER  { 
      $$ = malloc(strlen($1) + 15);
      sprintf($$, "%s %.3f", $1, $2);
      free($1);
    }

  | text_item PUNC_CHAR  { 
      $$ = malloc(strlen($1) + 5);
      sprintf($$, "%s %c", $1, $2);
      free($1);
    }
  ;


amount:
    // an empty amount - for one word timers
    LCURL RCURL {
      $$ = malloc(5);
      strcpy($$, "\0");
    }

  | LCURL NUMBER RCURL  { 
      // get string for amount
      $$ = malloc(100);
      sprintf($$, "%.3lf", $2);
    }

  | LCURL NUMBER UNIT RCURL { 
      $$ = malloc(strlen($3) + 20);
      sprintf($$, "%.3f %s", $2, $3);
      free($3);
    }

  | LCURL WORD RCURL  {
      $$ = $2;
    }

  | LCURL WORD UNIT RCURL {
      $$ = addTwoStrings($2, $3);
      free($2);
      free($3);
    }

  | LCURL MULTIWORD RCURL {
      $$ = $2;
    }

  | LCURL MULTIWORD UNIT RCURL {
      $$ = addTwoStrings($2, $3);
      free($2);
      free($3);
    }
  ;


cookware_amount:
    LCURL NUMBER RCURL  {
        $$ = malloc(100);
        sprintf($$, "%.3f", $2);
      }

  | LCURL WORD RCURL      {
        $$ = $2;
      }

  | LCURL MULTIWORD RCURL {
        $$ = $2;
    }
  ;

cookware:
  HWORD {
      $$ = strdup($1);
      addDirection(recipe, "Cookware", $1, NULL);
      free($1);
    }

  | HWORD LCURL RCURL {
      $$ = strdup($1);
      addDirection(recipe, "Cookware", $1, NULL);
      free($1);
    }

  | HWORD WORD LCURL RCURL  { 
      $$ = addTwoStrings($1, $2);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", valueString, NULL);
      free(valueString);
      free($1);
      free($2);
    }

  | HWORD MULTIWORD LCURL RCURL { 
      $$ = addTwoStrings($1, $2);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", valueString, NULL);
      free(valueString);
      free($1);
      free($2);
    }

  | HWORD cookware_amount  { 
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", $1, $2);
      free($1);
      free($2);
    }
  
  | HWORD WORD cookware_amount {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }
  
  | HWORD MULTIWORD cookware_amount {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }
  ;



ingredient:
    ATWORD  {
      $$ = strdup($1);
      addDirection(recipe, "Ingredient", $1, NULL);
      free($1);
    }

  | ATWORD amount {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Ingredient", $1, $2);
      free($1);
      free($2);
    }

  | ATWORD WORD amount  {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Ingredient", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }

  | ATWORD MULTIWORD amount  {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "Ingredient", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }
  ;


timer:
    TILDE amount  {
        $$ = strdup($2);
        addDirection(recipe, "Timer", NULL, $2);
        free($2);
      }
  | TILDE WORD {
        $$ = strdup($2);
        addDirection(recipe, "Timer", $2, NULL);
        free($2);
      }
  | TILDE WORD amount { 
        $$ = addTwoStrings($2, $3);
        addDirection(recipe, "Timer", $2, $3);
        free($2);
        free($3);
      }

  | TILDE MULTIWORD amount  { 
        $$ = addTwoStrings($2, $3);
        addDirection(recipe, "Timer", $2, $3);
        free($2);
        free($3);
      }
  ;


%%



int yyerror( Recipe * recipe, const char *s){
  printf("\nError\n%s", s);
}




#include "lex.yy.c"