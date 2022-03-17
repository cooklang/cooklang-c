
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

%define parse.error verbose

%expect 18


%parse-param {Recipe * recipe}


%union{
  char * string;
  char character;
  double number;
}

%token WORD MULTIWORD UNIT NUMBER LCURL RCURL PUNC_CHAR NL TILDE HWORD ATWORD METADATA COMMENT WHTS

%type <character> LCURL RCURL NL
%type <string> WORD MULTIWORD UNIT HWORD ATWORD METADATA PUNC_CHAR WHTS
%type <number> NUMBER

%type <string> text_item ingredient cookware timer amount
%type <string> input line step direction

%type <string> cookware_amount


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
  | step WHTS {
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s %s", $1, $2);

      addDirection(recipe, "text", $2, NULL);

      free($1);
      free($2);
    }
  ;


direction:
    text_item {
      $$ = strdup($1);
      addDirection(recipe, "text", $1, NULL);
      free($1);
    }
  | timer
  | cookware
  | ingredient
  | HWORD text_item   {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "cookware", $1, NULL);
      addDirection(recipe, "text", $2, NULL);
      free($1);
      free($2);
    }
  | ATWORD text_item  {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "ingredient", $1, NULL);
      addDirection(recipe, "text", $2, NULL);
      free($1);
      free($2);
    }
  | text_item WHTS {
    $$ = addTwoStrings($1, $2);
    char * tempString = addTwoStrings($1, $2);
    addDirection(recipe, "text", tempString, NULL);
    free(tempString);
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
  | PUNC_CHAR
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

  | text_item METADATA {
      $$ = addTwoStrings($1, $2);
      free($1);
      free($2);
    }
  ;


amount:
    // an empty amount - for one word timers
    LCURL RCURL {
      $$ = malloc(5);
      strcpy($$, "\0");
    }
  | LCURL WHTS RCURL {
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
    LCURL RCURL {
        $$ = malloc(5);
        strcpy($$, "\0");
      }

  | LCURL WHTS RCURL {
        $$ = malloc(5);
        strcpy($$, "\0");
      }

  | LCURL NUMBER RCURL  {
        $$ = malloc(100);
        sprintf($$, "%.3f", $2);
      }

  | LCURL WORD RCURL  {
        $$ = $2;
      }

  | LCURL MULTIWORD RCURL {
        $$ = $2;
      }
  ;

cookware:
  HWORD {
      $$ = strdup($1);
      addDirection(recipe, "cookware", $1, NULL);
      free($1);
    }

  | HWORD cookware_amount  {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "cookware", $1, $2);
      free($1);
      free($2);
    }

  | HWORD WORD cookware_amount {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "cookware", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }

  | HWORD MULTIWORD cookware_amount {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "cookware", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }
  ;



ingredient:
    ATWORD  {
      $$ = strdup($1);
      addDirection(recipe, "ingredient", $1, NULL);
      free($1);
    }

  | ATWORD amount {
      $$ = addTwoStrings($1, $2);
      addDirection(recipe, "ingredient", $1, $2);
      free($1);
      free($2);
    }

  | ATWORD WORD amount  {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "ingredient", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }

  | ATWORD MULTIWORD amount  {
      $$ = addThreeStrings($1, $2, $3);
      char * valueString = addTwoStrings($1, $2);
      addDirection(recipe, "ingredient", valueString, $3);
      free(valueString);
      free($1);
      free($2);
      free($3);
    }
  ;


timer:
    TILDE amount  {
        $$ = strdup($2);
        addDirection(recipe, "timer", NULL, $2);
        free($2);
      }
  | TILDE WORD {
        $$ = strdup($2);
        addDirection(recipe, "timer", $2, NULL);
        free($2);
      }
  | TILDE WORD amount {
        $$ = addTwoStrings($2, $3);
        addDirection(recipe, "timer", $2, $3);
        free($2);
        free($3);
      }

  | TILDE MULTIWORD amount  {
        $$ = addTwoStrings($2, $3);
        addDirection(recipe, "timer", $2, $3);
        free($2);
        free($3);
      }
  ;


%%




#include "lex.yy.c"



int yyerror( Recipe * recipe, const char * s){
  printf("\nError\n%s", s);
  return 1;
}
