#include "memory.h"
#include "kernel.h"
#include "tasklist.h"

extern struct Task * currentTask;
extern struct TaskList * runnableTasks;
extern struct TaskList * blockedTasks;
extern struct TaskList * allTasks;
extern struct Task *lowPriTask;
extern struct TaskList * deadTasks;
extern long canSwitch;
extern long pass;

unsigned char * oMemMax;
long nPagesFree;
long nPages;
struct MemStruct *firstFreeKMem;
/*static*/
long nextKPage;
struct MemStruct *firstFreeSharedMem;
unsigned short int *PMap;
long NoOfAllocations;
long memorySemaphore;

void
InitMem64(void)
{
   PMap = (unsigned short int *) PageMap;
   firstFreeKMem = (struct MemStruct *) 0x11000;
   firstFreeKMem->next = 0;
   firstFreeKMem->size = 0xFE0;
   firstFreeSharedMem = (struct MemStruct *) 0x1F0000;
   firstFreeSharedMem->next = 0;
   firstFreeSharedMem->size = 0xFE0;
   nextKPage = 0x12;
   currentTask = (struct Task *) TaskStruct;
   runnableTasks = (struct TaskList*)AllocKMem(sizeof(struct TaskList));
   runnableTasks->next = 0L;
   runnableTasks->task = currentTask;
   allTasks = (struct TaskList *)AllocKMem(sizeof(struct TaskList));
   allTasks->task = currentTask;
   allTasks->next = 0;
   deadTasks = 0;
   lowPriTask = 0L;
   blockedTasks = 0L;
   NoOfAllocations = 0;
   memorySemaphore = 0;
   canSwitch = 0;
   pass = 0;
}

//=========================================================================================
// Searches the linked list pointed to by list for a block of memory of size sizeRequested
// Allocates the memory and returns its address in RAX
//=========================================================================================
void *
AllocMem(long sizeRequested, struct MemStruct *list)
{
   unsigned char kernel = 0;
   if (list == firstFreeKMem || list == firstFreeSharedMem)
      kernel = 1;
   // We want the memory allocation to be atomic, so set a semaphore before proceeding
   SetSem(&memorySemaphore);
   while (list->size < sizeRequested)
   {
      if (list->next == 0)
      // Not enough memory available. Allocate another page.
      {
         long temp = (long) list >> 12;
         while (list->size < sizeRequested)
         {
            if (kernel)
               CreatePTE(AllocPage(1), ++temp << 12);
            else
               CreatePTE(AllocPage(currentTask->pid), ++temp << 12);
            list->size += PageSize;
         }
      }
      else
      {
         list = list->next;
      }
   }

   // We now have found a free memory block with enough (or more space)
   // Is there enough space for another link?
   if (list->size <= sizeRequested + sizeof(struct MemStruct))
   {
      // No. Just allocate the whole block
      list->size = 0;
   }
   else
   {
      // Yes, so create the new link
      void *temp = (void *) list;
      temp += sizeRequested;
      temp += sizeof(struct MemStruct);
      ((struct MemStruct *) temp)->next = list->next;
      list->next = (struct MemStruct *) temp;
      list->next->size = list->size - sizeRequested - sizeof(struct MemStruct);
      list->size = 0;
      list->pid = currentTask->pid;
   }
   ClearSem(&memorySemaphore);
   NoOfAllocations++;
   return (list + 1);
}

//==================================================
// Deallocate the memory at location list.
// This will deallocate both user and kernel memory
//==================================================
void
DeallocMem(void *list)
{
   struct MemStruct *l = (struct MemStruct *) list;

   // We want the memory deallocation to be atomic, so set a semaphore before proceeding
   SetSem(&memorySemaphore);
   l--;
   if (l->size == 0)
   {
      l->size = (long) l->next - (long) l - sizeof(struct MemStruct);
      NoOfAllocations--;
   }
   ClearSem(&memorySemaphore);
}

//===============================================================================
// Allocate some kernel memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *
AllocKMem(long sizeRequested)
{
   return (AllocMem(sizeRequested, firstFreeKMem));
}

//===============================================================================
// Allocate some user memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *
AllocUMem(long sizeRequested)
{
   return (AllocMem(sizeRequested, (void *) currentTask->firstfreemem));
}

//===============================================================================
// Allocate some shared memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *
AllocSharedMem(long sizeRequested)
{
   return (AllocMem(sizeRequested, firstFreeSharedMem));
}

//============================================================
// Deallocate shared memory belonging to a particular process.
//============================================================
void
DeallocSharedMem(long pid)
{
   struct MemStruct *l = firstFreeSharedMem;

   // We want the memory deallocation to be atomic, so set a semaphore before proceeding
   SetSem(&memorySemaphore);
   while (l->next != 0)
   {
      if (l->pid == pid)
      {
         l->size = (long) l->next - (long) l - sizeof(struct MemStruct);
         NoOfAllocations--;
      }
      l = l->next;
   }
   ClearSem(&memorySemaphore);
}

//============================================================
// Deallocate kernel memory belonging to a particular process.
//============================================================
void
DeallocKMem(long pid)
{
   struct MemStruct *l = firstFreeKMem;

   // We want the memory deallocation to be atomic, so set a semaphore before proceeding
   SetSem(&memorySemaphore);
   while (l->next != 0)
   {
      if (l->pid == pid)
      {
         l->size = (long) l->next - (long) l - sizeof(struct MemStruct);
         NoOfAllocations--;
      }
      l = l->next;
   }
   ClearSem(&memorySemaphore);
}
