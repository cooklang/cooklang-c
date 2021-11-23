
%{

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


#define YYDEBUG 1

int yylex();

int yyerror ( const char * s);

extern void yyrestart( FILE * input_file );

char string[20] = "header string";

%}

%union{
  char * string;
  char character;
  double number;
}

%token WORD MULTIWORD UNIT NUMBER LCURL RCURL PUNC_CHAR NL TILDE HWORD ATWORD

%type <string> WORD
%type <string> MULTIWORD
%type <string> UNIT
%type <number> NUMBER
%type <character> LCURL
%type <character> RCURL
%type <character> PUNC_CHAR
%type <string> HWORD
%type <string> ATWORD
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
  | input line  { printf("%s\n", $2); }
  ;


line:
    NL      {}
  | step NL {
      $$ = malloc(strlen($1) + 100);
      sprintf($$, "%s%c", $1, $2);
    }
  ;


step:
    direction           
  | step direction  { 
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }
  ;


direction:
    text_item         
  | timer             
  | cookware          
  | ingredient        
  | HWORD text_item   {
      // make directions in the step for each 
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }
  | ATWORD text_item  {
      // make directions in the step for each
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }
  ;



text_item:
    WORD
  | MULTIWORD
  | NUMBER  { 
      printf("text_item number\n");
      $$ = malloc(10);
      sprintf($$, "%.3f", $1);
    }

  | PUNC_CHAR { 
      $$ = malloc(5); 
      $$[0] = $1;
      $$[1] = '\0';
    }
  ;



// have to fix - add no amount input and no curls for one word timer
timer:
    TILDE amount  {
        $$ = $2;
      }

  | TILDE WORD amount { 
        // make timer string
        $$ = malloc(strlen($2) + strlen($3) + 10);
        sprintf($$, "%s %s", $2, $3);
        free($2);
        free($3);
      }

  | TILDE MULTIWORD amount  { 
        // make timer string
        $$ = malloc(strlen($2) + strlen($3) + 10);
        sprintf($$, "%s %s", $2, $3);
        free($2);
      }
  ;



amount:
    // an empty amount - for one word timers
    LCURL RCURL {
      $$ = malloc(5);
      $$[0] = ' ';
      $$[1] = '\0';
    }

  | LCURL NUMBER RCURL  { 
      // get string for amount
      $$ = malloc(100);
      sprintf($$, "%.3lf", $2);
    }

  | LCURL NUMBER UNIT RCURL { 
      // get string for amount
      // remove % from unit 
      $$ = malloc(100 + strlen($3) + 5);
      sprintf($$, "%.3lf %s", $2, $3); 
    }

  | LCURL WORD RCURL  {
      $$ = $2;
    }

  | LCURL WORD UNIT RCURL {
      $$ = malloc(strlen($2) + strlen($3) + 5);
      sprintf($$, "%s%s", $2, $3);
    }

  | LCURL MULTIWORD RCURL {
      $$ = $2;
    }

  | LCURL MULTIWORD UNIT RCURL {
      $$ = malloc(strlen($2) + strlen($3) + 5);
      sprintf($$, "%s%s", $2, $3);
    }
  ;



cookware:
  HWORD {
      // get string
      $$ = $1;
    }

  | HWORD LCURL RCURL {
      // get string
      $$ = $1;
    }

  | HWORD WORD LCURL RCURL  { 
      // get string
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }

  | HWORD MULTIWORD LCURL RCURL { 
      // get string
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }

  | HWORD cookware_amount  { 
      // get string
      $$ = $1;
    }

  | HWORD WORD cookware_amount { 
      // get string
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }

  | HWORD MULTIWORD cookware_amount  { 
      // get string
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
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


ingredient:
    ATWORD  {
      $$ = $1;
    }

  | ATWORD amount {
      $$ = $1;
    }

  | ATWORD WORD amount  {
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }

  | ATWORD MULTIWORD amount  {
      $$ = malloc(strlen($1) + strlen($2) + 5);
      sprintf($$, "%s%s", $1, $2);
    }

  ;




%%



int yyerror( const char *s){
  printf("\nError\n%s", s);
}




#include "lex.yy.c"