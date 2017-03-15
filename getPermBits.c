#include "getPermBits.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

/* --------------------------------- */
#define  REG_FILE "Regular File"
#define DIRECTORY "Directory"
#define  CHAR_DEV "Character Device"
#define   BLK_DEV "Block Device"
#define      FIFO "FIFO"
#define  SYM_LINK "Symbolic Link"
#define    SOCKET "Socket"
#define   UNKNOWN "Unknown?"
/* --------------------------------- */
#define SIZE 10
#define BIGS 'S'
#define LOWS 's'
#define LOWX 'x'
#define DASH '-'

void getPermissionBits(const char* fileName, char** savePermission)
{
   struct stat buf;
   char* fileType = NULL; 
   char permission[SIZE + 1];
   int type;

   /* If stat() doesn't work */
   if(stat(fileName, &buf))
   {
      perror(fileName);
      exit(-1);
   }
   
   type = getFileType(&fileType, &buf);
   getPermission(permission,type,&buf);
   
   *savePermission = strdup(permission);
}

void getPermission(char* access, int type, struct stat* fileInfo)
{
   int groupExBit;
   int userExBit;
   int setuidBit;
   int setgidBit;

   switch(type)
   {
      case 1: access[0] = 'b'; break;
      case 2: access[0] = 'c'; break;
      case 3: access[0] = 'd'; break;
      case 4: access[0] = 'l'; break; 
      case 5: access[0] = 's'; break;
      case 6: access[0] = 'p'; break;
      case 7: access[0] = '-'; break;
   }  

   if(fileInfo->st_mode & S_IXGRP)
      groupExBit = 1;
   else
      groupExBit = 0;

   if(fileInfo->st_mode & S_IXUSR)
      userExBit = 1;
   else
      userExBit = 0;

   if(fileInfo->st_mode & S_ISUID)
      setuidBit = 1;
   else
      setuidBit = 0;

   if(fileInfo->st_mode & S_ISGID)
      setgidBit = 1;
   else
      setgidBit = 0;

   access[1] = (fileInfo->st_mode & S_IRUSR) ? 'r' : '-'; /* user-read */
   access[2] = (fileInfo->st_mode & S_IWUSR) ? 'w' : '-'; /* user-write */
   access[3] = setExecuteBit(userExBit, setuidBit);       /* user-execute */
   access[4] = (fileInfo->st_mode & S_IRGRP) ? 'r' : '-'; /* group-read */
   access[5] = (fileInfo->st_mode & S_IWGRP) ? 'w' : '-'; /* group-write */
   access[6] = setExecuteBit(groupExBit, setgidBit);      /* group-exeucte */
   access[7] = (fileInfo->st_mode & S_IROTH) ? 'r' : '-'; /* other-read */
   access[8] = (fileInfo->st_mode & S_IWOTH) ? 'w' : '-'; /* other-write */
   access[9] = (fileInfo->st_mode & S_IXOTH) ? 'x' : '-'; /* other-execute */
   
   access[10] = '\0';
}

char setExecuteBit(int exBit, int idBit)
{
   if(exBit && idBit)
      return LOWS;
   else if(exBit == 0 && idBit == 0)
      return DASH;
   else if(exBit == 1 && idBit == 0)
      return LOWX;
   else if(exBit == 0 && idBit == 1)
      return BIGS;
   else
      return 0;
}

int getFileType(char** fileType, struct stat* fileInfo)
{
   if(S_ISREG(fileInfo->st_mode))
   {
      *fileType = REG_FILE;
      return 7;
   }
   else if(S_ISDIR(fileInfo->st_mode))
   {
      *fileType = DIRECTORY;
      return 3;
   }
   else if(S_ISCHR(fileInfo->st_mode))
   {
      *fileType = CHAR_DEV;
      return 2;
   }
   else if(S_ISBLK(fileInfo->st_mode))
   {
      *fileType = BLK_DEV;
      return 1;
   }
   else if(S_ISFIFO(fileInfo->st_mode))
   {
      *fileType = FIFO;
      return 6;
   }
   else if(S_ISLNK(fileInfo->st_mode))
   {
      *fileType = SYM_LINK;
      return 4;
   }
   /*else if(S_ISSOCK(fileInfo->st_mode))
   {
      *fileType = SOCKET;
      return 5;
   } */
   else
   {
      *fileType = UNKNOWN;
      return 0;
   }
}
