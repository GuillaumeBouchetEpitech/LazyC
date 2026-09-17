
#include "StreamWriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StreamWriter
{
  FILE *pFile;
} StreamWriter;

StreamWriter *StreamWriter__create(const char *inFilepath)
{
  StreamWriter *newStream = calloc(1, sizeof(StreamWriter));
  if (!newStream)
  {
    return NULL;
  }
  newStream->pFile = fopen(inFilepath, "w");
  if (!newStream->pFile)
  {
    free(newStream);
    return NULL;
  }
  return newStream;
}

int StreamWriter__write(StreamWriter *self, const char *inContentData, unsigned int inContentSize)
{
  char *inputPtr = (char*)inContentData;
  unsigned int totalToWrite = inContentSize;
  do {
    unsigned int totalWritten = fwrite(inputPtr, sizeof(char), totalToWrite, self->pFile);;

    if (totalWritten > 0)
    {
      totalToWrite -= totalWritten;
      inputPtr += totalWritten;
    }
    else {
      return -1;
    }
    // else if (errno != EINTR)
    // {
    //     goto out_error;
    // }
  }
  while (totalToWrite > 0);
  return 0;
}

void StreamWriter__free(StreamWriter **self)
{
  if (!self || !*self)
  {
    return;
  }
  // fflush((*self)->pFile);
  fclose((*self)->pFile);
  free(*self);
  *self = NULL;
}
