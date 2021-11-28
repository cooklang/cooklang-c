%{
#include <stdio.h>

#define YYDEBUG 1
  int yylex(void);
  void yyerror(const char* msg);
  extern FILE * yyin;

%}

%union{
  char * string;
  char character;
  double number;
}

%token UNKNOWN
%token AT TILDE HASH COLON
%token METADATA_BEGIN
%token MULTIWORD_WITH_LBRACE
%token WHITE_SPACE
%token LBRACE RBRACE
%token PERCENT
%token WORD
%token PUNCTUATION
%token NUMBER
%token MULTIWORD
%token PHRASE
%token NEW_LINE
%token COMMENT_LINE
%token COMMENT
%token END_OF_FILE 0

%type<string> WORD
%type<string> phrase
%type<string> MULTIWORD_WITH_LBRACE
%type<string> phrase_component

%start recipe
%%

recipe:         recipe_component
        |       recipe recipe_component
                ;

recipe_component: step recipe_component_ender {printf("=== New Step\n");}
                | metadata NEW_LINE {printf("=== New Metadata\n");}
                | NEW_LINE
                | recipe_component_ender
                ;

recipe_component_ender:
                NEW_LINE NEW_LINE
        |       NEW_LINE END_OF_FILE
                ;

metadata:       METADATA_BEGIN optional_white_space WORD COLON optional_white_space phrase {printf("%s is %s ",$3,$6);}
        ;

step:           step_component
        |       step step_component
                ;

step_component: WORD
        |       COMMENT_LINE {printf("COMMENT_LINE ");}
        |       COMMENT {printf("COMMENT ");}
        |       phrase {printf("PHRASE ");}
        |       ingredient {printf("INGREDIENT ");}
        |       cookware {printf("COOKWARE ");}
        |       timer {printf("TIMER ");}
                ;

phrase:      phrase_component
        |       phrase phrase_component {$$ = malloc(strlen($1)+strlen($2)+1); sprintf($$,"%s%s",$1,$2);}
                ;
phrase_component: WORD
        |       WHITE_SPACE {$$ = " ";}
        |       PUNCTUATION
        |       NUMBER
        |       PERCENT
        |       COLON {$$=":";}
                ;

ingredient:     AT WORD optional_white_space optional_ingredient_quantity
        |       AT MULTIWORD_WITH_LBRACE ingredient_quantity
        ;
optional_white_space:
        |       WHITE_SPACE
        ;
optional_ingredient_quantity:
        |        LBRACE ingredient_quantity
                ;

ingredient_quantity:
                |  optional_ingredient_quantity_unit RBRACE
        ;

optional_ingredient_quantity_unit:
                        | NUMBER optional_ingredient_unit
                                ;
optional_ingredient_unit:
        |       PERCENT WORD
        ;

cookware:       HASH WORD optional_white_space optional_cookware_quantity
        |       HASH MULTIWORD_WITH_LBRACE cookware_quantity
        ;

optional_cookware_quantity:
        |        LBRACE cookware_quantity
                ;
cookware_quantity: RBRACE // ONE
                | NUMBER RBRACE // GIVEN NUMBER
        ;

timer:           TILDE LBRACE NUMBER PERCENT WORD RBRACE
        ;



%%

void yyerror(const char* msg)
{
  printf("%s \n",msg);
}
