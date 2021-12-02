
%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "include/CooklangParser.h"

#define YYDEBUG 1

int yylex();

int yyerror ( Recipe * recipe, const char * s);

extern void yyrestart( FILE * input_file );

char string[20] = "header string";

// create the parser actually but thats not setup right now



%}

%parse-param {Recipe * recipe}


%union{
  char * string;
  char character;
  double number;
}

%token WORD MULTIWORD UNIT NUMBER LCURL RCURL PUNC_CHAR NL TILDE HWORD ATWORD METADATA

%type <string> WORD
%type <string> MULTIWORD
%type <string> UNIT
%type <number> NUMBER
%type <character> LCURL
%type <character> RCURL
%type <character> PUNC_CHAR
%type <string> HWORD
%type <string> ATWORD
%type <string> METADATA
%type <character> NL

%type <string> text_item
%type <string> step
%type <string> direction
%type <string> amount
%type <string> timer
%type <string> cookware
%type <string> cookware_amount ingredient line input


%define parse.error verbose


%%

input:
  %empty
  | input line  { }
  ;


line:
    NL      {}
  | step NL {
      $$ = malloc(strlen($1) + 100);
      sprintf($$, "%s%c", $1, $2);
      // after a step has been finished by a new line, have to add the step to the steplist
      // and make a new step to accept directions
      Step * newStep = createStep();

      insertBack(recipe->stepList, newStep);
    }
  | METADATA NL {
      printf("metadata: %S\n", $1);
    }
  ;


step:
    direction {
        printf("*New Step\n ***Direction: %s\n", $1);
      }
  | step direction {
      printf("***Direction: %s\n", $2);
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s %s", $1, $2);
    }
  ;


direction:
    text_item {
      addDirection(recipe, "TextItem", $1, NULL, -1, NULL);
    }
  | timer             
  | cookware          
  | ingredient        
  | HWORD text_item   {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Cookware", $1, NULL, -1, NULL);
      addDirection(recipe, "TextItem", $2, NULL, -1, NULL);
    }
  | ATWORD text_item  {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "Ingredient", $1, NULL, -1, NULL);
      addDirection(recipe, "TextItem", $2, NULL, -1, NULL);
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
    }
    
  | text_item MULTIWORD  { 
      $$ = addTwoStrings($1, $2);
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
    }

  | LCURL WORD RCURL  {
      $$ = $2;
    }

  | LCURL WORD UNIT RCURL {
      $$ = addTwoStrings($2, $3);
    }

  | LCURL MULTIWORD RCURL {
      $$ = $2;
    }

  | LCURL MULTIWORD UNIT RCURL {
      $$ = addTwoStrings($2, $3);
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
      $$ = $1;
    }

  | HWORD LCURL RCURL {
      $$ = $1;
    }

  | HWORD WORD LCURL RCURL  { 
      $$ = addTwoStrings($1, $2);
    }

  | HWORD MULTIWORD LCURL RCURL { 
      $$ = addTwoStrings($1, $2);
    }

  | HWORD cookware_amount  { 
      $$ = addTwoStrings($1, $2);
    }
  ;



ingredient:
    ATWORD  {
      $$ = $1;
    }

  | ATWORD amount {
      $$ = addTwoStrings($1, $2);
    }

  | ATWORD WORD amount  {
      $$ = addThreeStrings($1, $2, $3);
    }

  | ATWORD MULTIWORD amount  {
      $$ = addThreeStrings($1, $2, $3);
    }
  ;


timer:
    TILDE amount  {
        $$ = $2;
        // parse the amount
        char ** results = parseAmountString($2);

        addDirection(recipe, "Timer", $2, )
      }
  | TILDE WORD {
        $$ = $2;
        addDirection(recipe, "Timer", $2, NULL, -1, NULL);        
      }
  | TILDE WORD amount { 
        $$ = addTwoStrings($2, $3);
      }

  | TILDE MULTIWORD amount  { 
        $$ = addTwoStrings($2, $3);
      }
  ;


%%



int yyerror( Recipe * recipe, const char *s){
  printf("\nError\n%s", s);
}




#include "lex.yy.c"