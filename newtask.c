#include "kstructs.h"
#include "kernel.h"
#include "memory.h"
#include "filesystem.h"
#include "fat.h"

void
RemoveFromQ(struct Task *task, struct Task **QHead, struct Task **QTail);
void
AddToQ(struct Task *task, struct Task **QHead, struct Task **QTail);
void
StartTask();
struct Task *
NewKernelTask(void *TaskCode);

struct Task *currentTask = 0;
struct Task *runnableTasks[2] =
   { 0, 0 }; // [0] = Head, [1] = Tail
struct Task *blockedTasks[2] =
   { 0, 0 };
struct Task *lowPriTask = 0;
struct TaskList * allTasks = 0;
struct DeadTaskList *deadTasks;

extern long *tempstack;
extern long *tempstack0;
extern unsigned short int *PMap;
extern long nPagesFree;

static long nextpid = 3;

//============================================
//  Find the next free entry in the task table
//============================================
struct Task *
nextfreetss()
{
   struct Task *temp = (struct Task *) TaskStruct;

   while (temp->pid != 0)
   {
      temp++;
   }
   return (temp);
}

//===============================
// Link task into the task table
//===============================
void
LinkTask(struct Task *task)
{
   struct TaskList *tl = allTasks;

   task->nexttask = runnableTasks[0];
   runnableTasks[0] = task;
   if (tl->task)
   {
      while (tl->next)
      {
         tl = tl->next;
      }
   }
   tl->next = AllocKMem(sizeof(struct TaskList));
   tl->next->next = 0;
   tl->next->task = task;
}

//========================
// Create a new User task
//========================
void
NewTask(char *name, char *environment, struct MessagePort * parentPort,
      long console)
{
   long *stack;
   struct Task *task = nextfreetss();
   int result;

   task->pid = nextpid++;
   task->environment = environment;
   task->parentPort = parentPort;
   task->currentDir = currentTask->currentDir;
   task->console = console;
   task->waiting = 0;
   task->cr3 = (long) (VCreatePageDir(task->pid));
   task->ds = udata64 + 3;
   copyMem((unsigned char *) StartTask, (unsigned char *) TempUserCode,
         (long) NewKernelTask - (long) StartTask);
   copyMem(name, (unsigned char *) TempUserData, 100);
   stack = (long *) (TempUStack + PageSize) - 5;
   task->rsp = (long) ((long *) (UserStack + PageSize) - 5);
   task->r13 = (long) name;
   task->r15 = (long) task;
   stack[0] = UserCode;
   stack[1] = user64 + 3;
   stack[2] = 0x2202;
   stack[3] = UserStack + PageSize;
   stack[4] = udata64 + 3;

   asm ("cli");
   LinkTask(task);
   asm ("sti");
}

void
LoadTheProgram(long start, char * name)
{
   struct FCB * fHandle;
   long codelen, datalen;
   char header[3];
   long *data;
   long currentPage;
   long size;

   struct Message *FSMsg;

   FSMsg = (struct Message *) AllocKMem(sizeof(struct Message));

   // Open file
   FSMsg->nextMessage = 0;
   FSMsg->byte = OPENFILE;
   FSMsg->quad = (long) name;
   FSMsg->quad2 = (long) fHandle;
   SendReceiveMessage((struct MessagePort *) FSPort, FSMsg);

   fHandle = (struct FCB *) FSMsg->quad;
   if (fHandle)
   {
      ReadFromFile(fHandle, header, 4);
      ReadFromFile(fHandle, (char *) &codelen, 8);
      ReadFromFile(fHandle, (char *) &datalen, 8);
      currentPage = UserCode;
      size = codelen;
      while (codelen > PageSize)
      {
         CreatePTE(AllocPage(currentTask->pid), ++currentPage);
         size -= PageSize;
      }
      ReadFromFile(fHandle, (char *) UserCode, codelen);
      currentPage = UserData;
      size = datalen;
      while (datalen > PageSize)
      {
         CreatePTE(AllocPage(currentTask->pid), ++currentPage);
         size -= PageSize;
      }
      ReadFromFile(fHandle, (char *) UserData, datalen);
      data = (long *) (UserData + datalen);
      data[0] = 0;
      data[1] = PageSize - datalen - 0x10;
      currentTask->firstfreemem = UserData + datalen;

      //Close file
      FSMsg->nextMessage = 0;
      FSMsg->byte = CLOSEFILE;
      FSMsg->quad = (long) fHandle;
      SendReceiveMessage((struct MessagePort *) FSPort, FSMsg);
      DeallocMem(FSMsg);
      asm("mov $0x300000, %rcx");
   }
   else
   {
      DeallocMem(FSMsg);
      asm("mov $0x30000D, %rcx");
   }
}

//==================================
// Load, initialize, and start task
//==================================
void
StartTask()
{
   asm("mov $18, %r9;" // LOADPROGRAM
         "syscall;"
         "mov $13, %r9;" // KILLTASK
         "syscall;"
   );
}

//==========================
// Create a new Kernel task
//==========================
struct Task *
NewKernelTask(void *TaskCode)
{
   long *stack;
   struct Task *task = nextfreetss();
   long *data;

   task->pid = nextpid++;
   task->waiting = 0;
   task->cr3 = (long) VCreatePageDir(task->pid);
   task->ds = data64;
   stack = (long *) (TempUStack + PageSize) - 5;
   task->rsp = (long) ((long *) (UserStack + PageSize) - 5);
   task->r15 = (long) task;
   stack[0] = (long) TaskCode;
   stack[1] = code64;
   stack[2] = 0x2202;
   stack[3] = (long) UserStack + PageSize;
   stack[4] = data64;
   asm ("cli");
   LinkTask(task);
   data = (long *) TempUserData;
   data[0] = 0;
   data[1] = 0xFFE;
   task->firstfreemem = UserData;
   task->environment = (void *) 0;
   task->parentPort = (void *) 0;
   task->currentDir = currentTask->currentDir;
   task->console = 0;
   asm ("sti");
   return (task);
}

//=============================
// Create the low-priority task
//=============================
void
NewLowPriTask(void *TaskCode)
{
   lowPriTask = NewKernelTask(TaskCode);
   struct Task *temp = runnableTasks[0];
   RemoveFromQ(lowPriTask, &runnableTasks[0], &runnableTasks[1]);
   //    AddToQ(lowPriTask, &allTasks[0], &allTasks[1]);
}

//=======================
// Kill the current task
//=======================
void
KillTask(void)
{
   struct Task *task = currentTask;
   struct Task *temp = runnableTasks[0];

   if (task->parentPort)
   {
      struct Message *m = (struct Message *) AllocKMem(sizeof(struct Message));
      m->quad = 0;
      SendMessage(task->parentPort, m);
      DeallocMem(m);
   }

   //Don't want to task switch whilst destroying task
   asm ("cli");

   // Unlink task from runnable queue
   if (temp == task)
   {
      runnableTasks[0] = temp->nexttask;
      if (runnableTasks[0] == 0)
      {
         runnableTasks[1] = 0;
      }
   }
   else
   {
      while (temp)
      {
         if (temp->nexttask == task)
         {
            temp->nexttask = temp->nexttask->nexttask;
         }
         if (temp->nexttask == 0)
         {
            runnableTasks[1] = temp;
         }
         temp = temp->nexttask;
      }
   }

   // Add the task to the Dead Tasks queue
   // The dummy task will scan this queue and mark pages assigned by this task as free
   struct DeadTaskList * deadTask = AllocKMem(sizeof(struct DeadTaskList));
   deadTask->next = deadTasks;
   deadTask->pid = currentTask->pid;
   deadTasks = deadTask;

   struct TaskList * tl = allTasks;
   struct TaskList * tl1;
   if (tl->task == task)
   {
      allTasks->next = allTasks->next->next;
      allTasks->task = allTasks->next->task;
   }
   else
   {
      while (tl->next->task != task)
      {
         tl = tl->next;
      }
      tl1 = tl->next;
      tl->next = tl->next->next;
      DeallocMem(tl1);
   }

   // If there's any allocated shared memory, then free it
   DeallocSharedMem(task->pid);

   // If there's any allocated kernel memory, then free it
   // DeallocKMem(task->pid);
   // Careful!!! LinkTask() allocates kernel memory that is needed after the parent task has been killed.
   // Hopefully all kernel memory should be explicitly allocated, without need for this cleanup.

   // Reset PID so that OS knows the slot is free
   task->pid = 0;
   //    RemoveFromQ(task, &allTasks[0], &allTasks[1]);

   //SwTasks();
   SWTASKS;
}

//===============================================
// Move task from runnable queue to blocked queue
//===============================================
void
BlockTask(struct Task *task)
{
   RemoveFromQ(task, &runnableTasks[0], &runnableTasks[1]);
   AddToQ(task, &blockedTasks[0], &blockedTasks[1]);
}

//===============================================
// Move task from blocked queue to runnable queue
//===============================================
void
UnBlockTask(struct Task *task)
{
   RemoveFromQ(task, &blockedTasks[0], &blockedTasks[1]);
   AddToQ(task, &runnableTasks[0], &runnableTasks[1]);
}

//===========================================================
// Move the task at the head of the runnable queue to the end
//===========================================================
void
moveTaskToEndOfQueue()
{
   struct Task *temp = runnableTasks[0];

   if (temp->nexttask)
   {
      runnableTasks[0] = temp->nexttask;
      runnableTasks[1]->nexttask = temp;
      runnableTasks[1] = temp;
      temp->nexttask = 0;
   }
}

//========================
// Unlink task from queue
//========================
void
RemoveFromQ(struct Task *task, struct Task **QHead, struct Task **QTail)
{
   struct Task *temp = *QHead;

   if (temp == task)
   {
      *QHead = temp->nexttask;
      if (*QHead == 0)
      {
         *QTail = 0;
      }
   }
   else
   {
      while (temp)
      {
         if (temp->nexttask == task)
         {
            temp->nexttask = temp->nexttask->nexttask;
         }
         if (temp->nexttask == 0)
         {
            *QTail = temp;
         }
         temp = temp->nexttask;
      }
   }
}

//======================
// Put task on to queue
//======================
void
AddToQ(struct Task *task, struct Task **QHead, struct Task **QTail)
{
   if (*QHead == 0)
   {
      *QHead = *QTail = task;
      task->nexttask = 0;
   }
   else
   {
      (*QTail)->nexttask = task;
      *QTail = task;
      task->nexttask = 0;
   }
}

//=========================================
// Returns the task structure with PID pid
//=========================================
struct Task *
PidToTask(long pid)
{
   struct TaskList * tempTask = allTasks;

   while (tempTask)
   {
      if (tempTask->task->pid == pid)
         break;
      tempTask = tempTask->next;
   }
   return tempTask->task;
}

//=====================================
// The one task that is always runnable
//=====================================
void
dummyTask()
{
   while (1)
   {
      asm("hlt");
   }
}
