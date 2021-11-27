#include <stdio.h>
#include "cook_flexer.c"

int
main(int argc, char *argv[])
{
  --argc;
  ++argv;
  if (argc<1){
    fprintf(stderr, "No file provided\n");
    exit(EXIT_FAILURE);
  }

  cookin=fopen(argv[0],"r");
  if(!cookin){
    fprintf(stderr, "Could not open file %s\n",argv[0]);
    exit(EXIT_FAILURE);
  }

  if(cookparse()) exit(EXIT_FAILURE);

  printf("Someday I'll parse lots of recipes!\n");
  exit(EXIT_SUCCESS);
}
