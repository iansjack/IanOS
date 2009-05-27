#include "ckstructs.h"
#include "cmemory.h"
#include "filesystem.h"

//================================================
// Read noBytes into buffer from the file fHandle
//================================================
long ReadFromFile(struct FCB *fHandle, char *buffer, long noBytes)
{
   long retval;

   struct Message *FSMsg;

   FSMsg = (struct Message *)AllocKMem(sizeof(struct Message));
   char *buff = AllocKMem(noBytes);
   int  i;

   FSMsg->nextMessage = 0;
   FSMsg->byte        = READFILE;
   FSMsg->quad        = (long)fHandle;
   FSMsg->quad2       = (long)buff;
   FSMsg->quad3       = noBytes;
   SendReceiveMessage(FSPort, FSMsg);
   for (i = 0; i < noBytes; i++)
   {
      buffer[i] = buff[i];
   }
   DeallocMem(buff);
   retval = FSMsg->quad;
   DeallocMem(FSMsg);
   return(retval);
}


//===========================================
// A utility function to copy a memory range
//===========================================
void copyMem(unsigned char source[], unsigned char dest[], long size)
{
   int i;

   for (i = 0; i < size; i++)
   {
      dest[i] = source[i];
   }
}


//===========================================================================
// A kernel library function to write a null-terminated string to the screen.
//===========================================================================
void KWriteString(char *str, int row, int col)
{
   char *VideoBuffer = (char *)0xB8000;

   asm ("push %rax");
   asm ("push %rbx");
   asm ("push %rcx");
   asm ("push %rdx");
   asm ("push %rdi");
   asm ("push %rsi");
   int temp = 160 * row + 2 * col;
   int i    = 0;
   while (str[i] != 0)
   {
      VideoBuffer[temp + 2 * i] = str[i];
      i++;
   }
   asm ("pop %rsi");
   asm ("pop %rdi");
   asm ("pop %rdx");
   asm ("pop %rcx");
   asm ("pop %rbx");
   asm ("pop %rax");
}


//==========================================================================
// A kernel library function to write a quad character to the screen.
//==========================================================================
void KWriteHex(long c, int row) //, int col)
{
   char *VideoBuffer = (char *)0xB8000;

   asm ("push %rax");
   asm ("push %rbx");
   asm ("push %rcx");
   asm ("push %rdx");
   asm ("push %rdi");
   asm ("push %rsi");
   int i;
   for (i == 0; i < 8; i++)
   {
      char lo = c & 0xF;
      lo += 0x30;
      if (lo > 0x39)
      {
         lo += 7;
      }
      char hi = (c & 0xF0) >> 4;
      hi += 0x30;
      if (hi > 0x39)
      {
         hi += 7;
      }
      VideoBuffer[160 * row + 28 - 4 * i] = hi;
      VideoBuffer[160 * row + 30 - 4 * i] = lo;
      c = c >> 8;
   }
   asm ("pop %rsi");
   asm ("pop %rdi");
   asm ("pop %rdx");
   asm ("pop %rcx");
   asm ("pop %rbx");
   asm ("pop %rax");
}
