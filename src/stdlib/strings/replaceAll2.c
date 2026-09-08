
#include "./replaceAll2.h"

#include <stdlib.h>  // <- calloc(...)
#include <stdio.h>  // <- sprintf(...)
#include <string.h> // <- strstr(...), strlen(...)
// #include <stddef.h> // <- size_t

typedef struct StrLink
{
  char* match;
  struct StrLink* next;
}
StrLink;

StringData String__replaceAll2(
  const char *inContent,
  const char *inPattern,
  const char *inReplacement)
{

  // register the occurrences
  // determine the right size of the output buffer
  // allocate and append the content to the output buffer

  const unsigned int patternLen = strlen(inPattern);

  unsigned int totalLinks = 0;
  StrLink* headLink = NULL;
  StrLink* tailLink = NULL;

  const char *cursor = inContent;
  for (;;)
  {
    char *match = strstr(cursor, inPattern);
    if (match == NULL)
    {
      break;
    }

    StrLink* newLink = calloc(1, sizeof(StrLink));
    newLink->match = match;
    newLink->next = NULL;
    if (!headLink) {
      headLink = newLink;
    }
    if (tailLink) {
      tailLink->next = newLink;
    }
    tailLink = newLink;
    totalLinks += 1;

    cursor = match + patternLen;
  }

  const unsigned int contentLen = strlen(inContent);
  if (totalLinks == 0) {

    // TODO: inefficient...

    // reply a duplicated of the original
    StringData newData;
    newData.data = strdup(inContent);
    newData.len = contentLen;
    return newData;
  }

  // const unsigned int contentLen = strlen(inContent);
  const unsigned int replacementLen = strlen(inReplacement);
  const int diffLen = ((int)replacementLen) - ((int)patternLen);

  // printf("diffLen %d\n", diffLen);

  unsigned int newLen = contentLen + (unsigned int)(((int)totalLinks) * diffLen);

  // printf("newLen %d\n", newLen);

  StringData newData;
  newData.data = calloc(newLen + 1, sizeof(char));
  newData.len = newLen;

  // unsigned int currentPos = 0;
  StrLink* currLink = headLink;

  const char *readCursor = inContent;
  char *writeCursor = newData.data;
  while (currLink)
  {
    // printf("currLink->match %s\n", currLink->match);

    // append what is before the link
    {
      unsigned int len = (int)(currLink->match - readCursor);

      // printf("len %d\n", len);

      memcpy(writeCursor, readCursor, len);
      readCursor += len;
      writeCursor += len;
    }

    // append the replacement
    {
      // printf("replacementLen %d\n", replacementLen);
      memcpy(writeCursor, inReplacement, replacementLen);
      readCursor += patternLen;
      writeCursor += replacementLen;
    }

    // advance
    StrLink* toFree = currLink;
    currLink = currLink->next;
    free(toFree);
  }

  {
    unsigned int len = (int)((inContent + contentLen) - readCursor);
    memcpy(writeCursor, readCursor, len);
    // readCursor += len;
    // writeCursor += len;
  }

  return newData;
}

