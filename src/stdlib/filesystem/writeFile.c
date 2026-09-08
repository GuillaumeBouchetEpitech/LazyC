
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int writeFile(const char* inFilepath, const char* inFileContent, unsigned int inFileSize)
{
  FILE* pFile = fopen(inFilepath, "w");
  if (!pFile) {
    return -1;
  }
  fwrite(inFileContent, sizeof(char), inFileSize, pFile);
  fflush(pFile);
  fclose(pFile);
  return 0;
}
