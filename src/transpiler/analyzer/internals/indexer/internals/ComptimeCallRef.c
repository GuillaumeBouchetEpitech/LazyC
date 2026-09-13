
#include "./ComptimeCallRef.h"

#include "stdlib/strings/trimStr.h"
#include "stdlib/strings/StringBuffer.h"
#include "stdlib/strings/replaceAll.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int _isComptimeType(const char* inTypename)
{
  return strchr(inTypename, '<') != NULL;
}

ComptimeCallRef ComptimeCallRef__create(
    NodePos inStartPos,
    NodePos inEndPos,
    const char *inVarName,
    const char *inArgsValue
) {
  ComptimeCallRef newComptimeCallRef;
  memset(&newComptimeCallRef, 0, sizeof(ComptimeCallRef));

  newComptimeCallRef.startPos = inStartPos;
  newComptimeCallRef.endPos = inEndPos;

  newComptimeCallRef.varName = strdup(inVarName);
  newComptimeCallRef.argsValue = strdup(inArgsValue);

  //
  // extract args as list
  //

  newComptimeCallRef.argsList = HeapArray<StringData>::preAllocated(32);


  unsigned int argsLen = strlen(inArgsValue);
  const char* startStr = inArgsValue + 1; // ignore the '<'
  unsigned int newLength = argsLen - 2; // ignore the '>'

  if (newLength > 0) {
    char* tmpArgsValue = strndup(startStr, newLength);



    unsigned int savedIndex = 0;
    unsigned int comptimeDepth = 0;
    for (unsigned int ii = 0; ii < newLength; ++ii)
    {
      if (tmpArgsValue[ii] == ',' && comptimeDepth == 0)
      {
        // new arg
        char* newArgStr = strndup(tmpArgsValue + savedIndex, ii - savedIndex);
        // printf("   -> newArgStr %s\n", newArgStr);

        char* trimmedStrPtr = String__trim(newArgStr);
        char* trimmedStr = strdup(trimmedStrPtr);

        // printf("     -> trimmedStr %s\n", trimmedStr);

        free(newArgStr);

        if (_isComptimeType(trimmedStr))
        {
          // TODO

          const char* delim = strchr(trimmedStr, '<');

          unsigned int nameLen = (int)(delim - trimmedStr);
          char* nameStr = strndup(trimmedStr, nameLen);

          // printf(" -===-> nameStr: %s\n", nameStr);

          unsigned int argLen = (int)(strlen(trimmedStr) - nameLen);
          char* argStr = strndup(delim, argLen);

          // printf(" -===-> argStr: %s\n", argStr);

          NodePos fakeNodePos;
          ComptimeCallRef tmpComptimeCall = ComptimeCallRef__create(fakeNodePos, fakeNodePos, nameStr, argStr);

          free(trimmedStr);
          trimmedStr = strdup(tmpComptimeCall.signature);

          // printf(" -===-> trimmedStr: %s\n", trimmedStr);

          ComptimeCallRef__free(&tmpComptimeCall);

          free(argStr);
          free(nameStr);
        }

        // PointerHeapArray__pushBack(newComptimeCallRef.argsList, trimmedStr);

        StringData tmpData;
        tmpData.data = trimmedStr;
        tmpData.len = strlen(trimmedStr);
        HeapArray<StringData>::pushBackRef(&newComptimeCallRef.argsList, &tmpData);

        savedIndex = ii + 1;
      }
      else if (tmpArgsValue[ii] == '<')
      {
        comptimeDepth += 1;
      }
      else if (tmpArgsValue[ii] == '>')
      {
        comptimeDepth -= 1;
      }
    }

    {
      // new arg
      char* newArgStr = strndup(tmpArgsValue + savedIndex, newLength - savedIndex);
      // printf("   -> newArgStr %s\n", newArgStr);

      char* trimmedStrPtr = String__trim(newArgStr);
      char* trimmedStr = strdup(trimmedStrPtr);

      // printf("     -> trimmedStr %s\n", trimmedStr);

      free(newArgStr);

      if (_isComptimeType(trimmedStr))
      {
        // TODO

        const char* delim = strchr(trimmedStr, '<');

        unsigned int nameLen = (int)(delim - trimmedStr);
        char* nameStr = strndup(trimmedStr, nameLen);

        // printf(" -===-> nameStr: %s\n", nameStr);

        unsigned int argLen = (int)(strlen(trimmedStr) - nameLen);
        char* argStr = strndup(delim, argLen);

        // printf(" -===-> argStr: %s\n", argStr);

        NodePos fakeNodePos;
        ComptimeCallRef tmpComptimeCall = ComptimeCallRef__create(fakeNodePos, fakeNodePos, nameStr, argStr);

        free(trimmedStr);
        trimmedStr = strdup(tmpComptimeCall.signature);

        // printf(" -===-> trimmedStr: %s\n", trimmedStr);

        ComptimeCallRef__free(&tmpComptimeCall);

        free(argStr);
        free(nameStr);
      }

      // PointerHeapArray__pushBack(newComptimeCallRef.argsList, trimmedStr);

      StringData tmpData;
      tmpData.data = trimmedStr;
      tmpData.len = strlen(trimmedStr);
      HeapArray<StringData>::pushBackRef(&newComptimeCallRef.argsList, &tmpData);

      // savedIndex = ii + 1;
    }


    free(tmpArgsValue);
  }

  {
    StringBuffer* stringBuffer = StringBuffer__create();
    StringBuffer__appendCString(stringBuffer, newComptimeCallRef.varName);
    StringBuffer__appendCString(stringBuffer, "__");

    for (unsigned int ii = 0; ii < newComptimeCallRef.argsList.len; ++ii)
    {
      const StringData* argStr = &newComptimeCallRef.argsList.data[ii];

      unsigned int bufferLen = argStr->len * 3;
      char* tmpBuffer = calloc(bufferLen, sizeof(char));
      String__replaceAll(argStr->data, "*", "Ptr", tmpBuffer, bufferLen);

      StringBuffer__appendCString(stringBuffer, tmpBuffer);
      free(tmpBuffer);
    }

    newComptimeCallRef.signature = strdup(stringBuffer->data);

    StringBuffer__free(&stringBuffer);
  }

  return newComptimeCallRef;
}

void ComptimeCallRef__free(ComptimeCallRef *self)
{
  if (!self)
  {
    return;
  }

  free(self->signature);

  for (unsigned int ii = 0; ii < self->argsList.len; ++ii)
  {
    StringData* argStr = &self->argsList.data[ii];
    free(argStr->data);
  }
  HeapArray<StringData>::free(&self->argsList);
  // PointerHeapArray__free((self->argsList);

  free(self->argsValue);
  free(self->varName);

  memset(self, 0, sizeof(ComptimeCallRef));
}
