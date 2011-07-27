#include "kstructs.h"
#include "kernel.h"
#include "memory.h"
#include "filesystem.h"
#include "fat.h"
#include "tasklist.h"

void
StartTask();
struct Task *
NewKernelTask(void *TaskCode);
long
ParseEnvironmentString(long *);

struct Task * currentTask = 0;
struct TaskList * runnableTasks = 0;
struct TaskList * blockedTasks = 0;
struct Task * lowPriTask = 0;
struct TaskList * allTasks = 0;
struct TaskList * deadTasks;

extern long *tempstack;
extern long *tempstack0;
extern unsigned short int *PMap;
extern long nPagesFree;
extern long nPages;

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
   asm("cli");
   allTasks = AddToTailOfTaskList(allTasks, task);
   runnableTasks = AddToTailOfTaskList(runnableTasks, task);
   asm("sti");
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
   task->r13 = (long) TempUserData; //name;
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

//===============================================================================
// This loads the program "name" into memory, if it exists.
// It is never called directly, but only as part of the System Call LOADPROGRAM.
// It sets RCX to either the start of the program (if it has been loaded)
// or to the KILLTASK System Call if the program didn't exist.
//===============================================================================
void
LoadTheProgram(long start, char * name)
{
   struct FCB * fHandle;
   long codelen, datalen;
   char header[3];
   long *data;
   long currentPage;
   long size;
   long argc;
   long argv;

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
      char * env = currentTask->environment;
      if (env)
      {
         char * newenv = AllocMem(81,
               (struct MemStruct *) currentTask->firstfreemem);
         copyMem(env, newenv, 81);
         DeallocMem(env);
         currentTask->environment = newenv;
         argc = ParseEnvironmentString(&argv);
         asm ("mov %0,%%rdi;"
               "mov %1,%%rsi"
               :
               :"r"(argc), "r"(argv)
               :"%rax","%rdi"
         );

      }
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
   runnableTasks = RemoveFromTaskList(runnableTasks, lowPriTask);
   lowPriTask->nexttask = (struct Task *) 0;
}

//=======================
// Kill the current task
//=======================
void
KillTask(void)
{
   struct Task *task = currentTask;

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
   runnableTasks = RemoveFromTaskList(runnableTasks, task);

   // Add the task to the Dead Tasks queue
   deadTasks = AddToHeadOfTaskList(deadTasks, currentTask);

   // Remove task from allTasks queue;
   allTasks = RemoveFromTaskList(allTasks, currentTask);

   task->pid = 0;
   SWTASKS;
}

//===============================================
// Move task from runnable queue to blocked queue
//===============================================
void
BlockTask(struct Task *task)
{
	asm("cli");
	runnableTasks = RemoveFromTaskList(runnableTasks, task);
	blockedTasks = AddToTailOfTaskList(blockedTasks, task);
	asm("sti");
}

//===============================================
// Move task from blocked queue to runnable queue
//===============================================
void
UnBlockTask(struct Task *task)
{
	asm("cli");
	blockedTasks = RemoveFromTaskList(blockedTasks, task);
	runnableTasks = AddToTailOfTaskList(runnableTasks, task);
	asm("sti");
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
   unsigned short int pid;
   struct TaskList * t;
   int count;

   while (1)
   {
      if (deadTasks)
      {
         t = deadTasks;
         deadTasks = deadTasks->next;
         pid = t->task->pid;
         DeallocMem(t);
         for (count = 0; count < nPages; count++)
         {
            if (PMap[count] == pid)
            {
               PMap[count] = 0;
               nPagesFree++;
            }
         }
         //DeallocSharedMem(pid);
         // DeallocKMem(task->pid);
         // Careful!!! LinkTask() allocates kernel memory that is needed after the parent task has been killed.
         // Hopefully all kernel memory should be explicitly allocated, without need for this cleanup.
      }
      else
         asm("hlt");
   }
}

long
ParseEnvironmentString(long * l)
{
   long argc = 0;
   int count = 0;

   char * env = currentTask->environment;

   *l = (long)AllocMem(80, (struct MemStruct *) currentTask->firstfreemem);
   long * argv = (long *) *l;
   argv[0] = (long)env;
   while (env[count])
   {
      while (env[count] && env[count] != ' ')
      {
         count++;
      }
      argc++;
      if (env[count])
      {
         env[count] = 0;
         argv[argc] = (long)env + ++count;
      }
   }
   return argc;
}

