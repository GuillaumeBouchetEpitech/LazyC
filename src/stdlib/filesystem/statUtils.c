
#include "./statUtils.h"

#include <stdio.h>
#include <sys/stat.h>

int Stat__pathExist(const char* inPath)
{
  struct stat statbuf;

  return (stat(inPath, &statbuf) == 0) ? 1 : 0;
}

