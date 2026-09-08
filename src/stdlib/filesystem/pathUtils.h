
#pragma once

char* Path__dirname(const char* inPath);
char* Path__basename(const char* inPath);
char* Path__extname(const char* inPath);
char* Path__normalize(const char* joined);
char *Path__join(unsigned int n, ...);
char *Path__relative(const char *from, const char *to);

