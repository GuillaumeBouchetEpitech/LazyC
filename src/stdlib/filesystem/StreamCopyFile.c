
#include "StreamCopyFile.h"

#include "./StreamReader.h"
#include "./StreamWriter.h"

#include <stdlib.h>

int StreamCopyFile__copyFile(const char *inFromFilepath, const char *inToFilepath)
{
  int result = 0;
  StreamReader* streamReader = StreamReader__create(inFromFilepath);
  StreamWriter* streamWriter = StreamWriter__create(inToFilepath);

  const unsigned int k_bufferSize = 1024;
  char* tmpBuffer = malloc(k_bufferSize * sizeof(char));

  if (!tmpBuffer || !streamWriter || !streamReader)
  {
    free(tmpBuffer);
    StreamWriter__free(&streamWriter);
    StreamReader__free(&streamReader);
    return -1;
  }

  do {

    unsigned int totalRead = StreamReader__read(streamReader, tmpBuffer, k_bufferSize);

    if (
      totalRead > 0 &&
      StreamWriter__write(streamWriter, tmpBuffer, totalRead) < 0
    ) {
      result = -1;
      break;
    }

    if (totalRead < k_bufferSize) {
      break;
    }
  }
  while (1);

  free(tmpBuffer);
  StreamWriter__free(&streamWriter);
  StreamReader__free(&streamReader);
  return result;
}
