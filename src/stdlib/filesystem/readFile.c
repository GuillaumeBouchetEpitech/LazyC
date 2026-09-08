
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readFile(const char* inFilepath, char** ppOutFileContent, unsigned int* pOutFileSize)
{
  FILE* pFile = fopen(inFilepath, "rb");
  if (pFile == NULL) {
    return -1;
  }

  // determine the file size
  fseek(pFile, 0, SEEK_END);
  long fileSize = ftell(pFile);
  if (fileSize < 0) {
    fclose(pFile);
    return -1;
  }
  fseek(pFile, 0, SEEK_SET);

  char* pFileContent = calloc((size_t)(fileSize + 1), sizeof(char));
  if (!pFileContent) {
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

