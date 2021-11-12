#include <stdio.h>

#define test(function) test_helper(#function,function)
int
test_helper(const char * message,int (*func) ())
{
  if(func()){
   printf("\x1b[32m\"%s\" Passed\x1b[0m\n",message);
   return 1;
  }
  else {
    printf("\x1b[31m\"%s\" Failed\x1b[0m\n",message);
    return 0;
  }
}

int a_failing_test()
{
  int result=0;
  return result;
}

int a_passing_test()
{
  int result=1;
  return result;
}


int main(int argc, char *argv[]) {
  int ret=1;
  ret &= test(a_passing_test);
  ret &= test(a_failing_test);
  return !ret;
}
