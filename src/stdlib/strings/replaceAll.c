
#include "./replaceAll.h"

#include <stdio.h>  // <- sprintf(...)
#include <string.h> // <- strstr(...), strlen(...)
#include <stddef.h> // <- size_t

unsigned int String__replaceAll(
    const char *inContent,
    const char *inPattern,
    const char *inReplacement,
    char *outBuffer,
    unsigned int outSize)
{
  const unsigned int patternLen = strlen(inPattern);
  const unsigned int replacementLen = strlen(inReplacement);
  const unsigned int maxOut = outSize > 0 ? outSize - 1 : 0;
  unsigned int requiredLen = 0;
  unsigned int written = 0;

  if (patternLen == 0 || strcmp(inPattern, inReplacement) == 0)
  {
    requiredLen = strlen(inContent);
    if (maxOut > 0)
    {
      unsigned int toCopy = requiredLen < maxOut ? requiredLen : maxOut;
      memcpy(outBuffer, inContent, toCopy);
      outBuffer[toCopy] = '\0';
    }
    return requiredLen;
  }

  const char *cursor = inContent;

  for (;;)
  {
    const char *match = strstr(cursor, inPattern);
    if (match == NULL)
    {
      break;
    }

    const unsigned int gapLen = (unsigned int)(match - cursor);
    requiredLen += gapLen;
    if (written < maxOut)
    {
      unsigned int avail = maxOut - written;
      unsigned int n = gapLen < avail ? gapLen : avail;
      memcpy(outBuffer + written, cursor, n);
      written += n;
    }

    requiredLen += replacementLen;
    if (written < maxOut)
    {
      unsigned int avail = maxOut - written;
      unsigned int n = replacementLen < avail ? replacementLen : avail;
      memcpy(outBuffer + written, inReplacement, n);
      written += n;
    }

    cursor = match + patternLen;
  }

  const unsigned int tailLen = strlen(cursor);
  requiredLen += tailLen;
  if (outSize > 0)
  {
    if (written < maxOut)
    {
      unsigned int avail = maxOut - written;
      unsigned int n = tailLen < avail ? tailLen : avail;
      memcpy(outBuffer + written, cursor, n);
      written += n;
    }
    outBuffer[written] = '\0';
  }

  return requiredLen;
}
