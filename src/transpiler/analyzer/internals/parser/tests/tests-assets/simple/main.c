
#include <stdlib.h>
#include <stdio.h>

void my_func();

int main(int argc, char** argv)
{
  my_func();
  return EXIT_SUCCESS;
}

void my_func()
{
  printf("Hello world!\n");
}
