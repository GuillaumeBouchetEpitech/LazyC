
#include "./readFile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int readFile(const char *inFilepath, char **ppOutFileContent, unsigned int *pOutFileSize)
{
  struct stat statbuf;
  if (stat(inFilepath, &statbuf) != 0)
  {
    return -1;
  }

  FILE *pFile = fopen(inFilepath, "rb");
  if (pFile == NULL)
  {
    return -1;
  }

  const long fileSize = statbuf.st_size;

  char *pFileContent = malloc((size_t)fileSize + 1);
  if (!pFileContent)
  {
    fclose(pFile);
    return -1;
  }
  size_t nread = fread(pFileContent, 1, (size_t)(fileSize), pFile);
  if (nread != (size_t)(fileSize))
  {
    free(pFileContent);
    fclose(pFile);
    return -1;
  }
  pFileContent[fileSize] = '\0';
  fclose(pFile);

  *ppOutFileContent = pFileContent;
  *pOutFileSize = fileSize;
  return 0;
}
