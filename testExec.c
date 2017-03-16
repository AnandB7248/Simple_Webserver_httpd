#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
   if(execl("/bin/cat", "cat", "main.c", (char*) NULL) < 0)
   {
      perror("execlp");
      exit(-1);
   }

   return 0;
}
