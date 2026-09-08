
#include "StreamReader.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StreamReader
{
  FILE *pFile;
} StreamReader;

StreamReader *StreamReader__create(const char *inFilepath)
{
  StreamReader *newStream = calloc(1, sizeof(StreamReader));
  if (!newStream)
  {
    return NULL;
  }
  newStream->pFile = fopen(inFilepath, "r");
  if (!newStream->pFile)
  {
    free(newStream);
    return NULL;
  }
  return newStream;
}

unsigned int StreamReader__read(StreamReader *self, char *outBufferData, unsigned int inBufferSize)
{
  // return fread(outBufferData, sizeof(char), inBufferSize, self->pFile);

  char *outputPtr = outBufferData;
  unsigned int totalToRead = inBufferSize;
  do {
    unsigned int totalRead = fread(outputPtr, sizeof(char), totalToRead, self->pFile);

    if (totalRead > 0)
    {
      totalToRead -= totalRead;
      outputPtr += totalRead;
    }
    else
    {
      break;
    }
  }
  while (totalToRead > 0);

  return inBufferSize - totalToRead;
}

void StreamReader__free(StreamReader **self)
{
  if (!self || !*self)
  {
    return;
  }
  fclose((*self)->pFile);
  free(*self);
  *self = NULL;
}
