#include "dynamicStrCat.h"
#include <string.h>
#include <stdlib.h>

char* dynamicStrCat(char* str1, char* str2)
{
   int len1, len2, lenComb;
   char* combinedStr;

   if((str1 == NULL) || (str1 == NULL))
      return NULL;

   len1 = strlen(str1);
   len2 = strlen(str2);
   lenComb = len1 + len2;
   combinedStr = (char*) malloc(lenComb + 1);

   combinedStr = strncat(combinedStr, str1, len1);
   combinedStr = strncat(combinedStr, str2, len2);
   combinedStr[lenComb] = '\0';

   return combinedStr;
}
