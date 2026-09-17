
#include "./panic.h"

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void panic(const char* inErrMsg)
{
  fprintf(stderr, "\nPANIC\n-> \"%s\"\n", inErrMsg);
  raise(SIGINT);
  exit(1);
}

