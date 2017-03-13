/* CPE 357 - Keen           */
/* ------------------------ */
/* Student: Anand Batjargal */
/* Assignment: Lab #3       */

#include "readline.h"
#include <stdio.h>
#include <stdlib.h>
   /* This function assumes that the file pointer f, points to an existing file where we can read from.*/
char *readline(FILE *f)
{
   int size = 128;         /* Variable to hold the size of an array. */
                           /* Will be needed if it is necessary to double the array later on. */
   char *array = (char*) malloc(size * sizeof(char));          /* Dynamically allocated array to hold the individual characters */ 
                                                               /* read in from f. */
   int index = 0;          /* Create an int index to help place characters into the above array. */
   char temp;              /* Create a temp char to hold the individual characters from the file. */
   
   /* Use a while loop, to place the characters from the file until EOF or newline is encountered. */
   while(((temp = fgetc(f)) != EOF) && (temp != '\n') )
   {
      /* Check if size limit is reached. If so, double the size of the array. */
      if(index == size)
      {
         size = 2 * size;
         array = (char*) realloc(array, size);
         
         /* Check if realloc worked */
         if(array == NULL)
         {
            /* QUESTION: What are the necessary actions to take if realloc failed to work? */
            perror("Error: ");
            free(array);
            /* exit(-1); */
            return NULL;
         }
      }

      /* Place the character from the file into the string (char array) */
      array[index++] = temp;
   }

   /* Now that EOF or newline has been encountered, we stop placing the characters from the file to the string. */
   /* So we should go ahead and put the null terminator '\0' to the end of the string */
   
   /* Before we do that, we should check if placing null terminator will violate the size limit of the current array. */
   if(index == size)
   {
      size = size + 1;
      array = (char*) realloc(array,size);
      
      /* Check if realloc worked */
      if(array == NULL)
      {
         /* QUESTION: What are the necessary actions to take if realloc failed to work? */
         perror("Error: ");
         free(array);
         /* exit(-1); */
         return NULL;
      }
   }
   
   if(index == 0)
   {
      free(array);
      return NULL;
   }
   else
   {   
      /* Place the null terminator at the end of the string (char array). */
      array[index] = '\0';
   
      /* QUESTION: So returning array, a dynamically allocated memory, who is responsible to free it? */
      return array;
   }
}
