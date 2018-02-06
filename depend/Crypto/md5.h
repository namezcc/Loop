#ifndef MD5_H
#define MD5_H

#include <stdio.h>

// Size of MD5
#define MD5_SIZE 32
#ifdef  __cplusplus
extern  "C" {
#endif
// ---------- Includes ------------
char * MD5_string (char *string);
void   MD5_stringEx (char *string, char *buffer);
char * MD5_file   (FILE *file);
#ifdef  __cplusplus
}
#endif

#endif