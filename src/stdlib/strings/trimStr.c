
#include "./trimStr.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

char* String__ltrim(char* str) {
  while (isspace((unsigned char)*str)) {
    str++;
  }
  return str;
}

char* String__rtrim(char* str) {
  char* end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    end--;
  }
  *(end + 1) = '\0';
  return str;
}

char* String__trim(char* str) {
  return String__rtrim(String__ltrim(str));
}
