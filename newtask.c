#include <kernel.h>
#include <filesystem.h>

struct Task *
NewKernelTask(void *TaskCode);
long ParseEnvironmentString(long *);

struct Task *currentTask;
struct TaskList *runnableTasks;
struct TaskList *blockedTasks;
struct Task *lowPriTask;
struct TaskList *allTasks;
struct TaskList *deadTasks;
long canSwitch;
long pass;

extern unsigned short int *PMap;
extern long nPagesFree;
extern long nPages;
extern long firstFreePage;
extern struct MessagePort *FSPort;

long nextpid;

//===============================
// Link task into the task table
//===============================
void LinkTask(struct Task *task)
{
	asm("cli");
	allTasks = AddToTailOfTaskList(allTasks, task);
	runnableTasks = AddToTailOfTaskList(runnableTasks, task);
	asm("sti");
}

//=========================================================================
// Fork the current process.
// Return the pid of the new process
//=========================================================================
long DoFork()
{
	// Copy task structure, with adjustments
	struct Task *task = (struct Task *) AllocKMem(sizeof(struct Task));
	copyMem((unsigned char *) currentTask, (unsigned char *) task,
			sizeof(struct Task));
	int pid = task->pid = nextpid++;
	task->currentDirName = AllocKMem(strlen(currentTask->currentDirName) + 1);
	strcpy(task->currentDirName, currentTask->currentDirName);
	task->parentPort = 0;

	// Copy Page Table and pages
	task->cr3 = (long) VCreatePageDir(pid, currentTask->pid);
	task->forking = 1;

	// Create FCBs for STDI, STDOUT, and STDERR

	// STDIN
	struct FCB *fcbin = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcbin->fileDescriptor = STDIN;
	fcbin->deviceType = KBD;
	task->fcbList = fcbin;

	// STDOUT
	struct FCB *fcbout = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcbout->fileDescriptor = STDOUT;
	fcbout->deviceType = CONS;
	fcbin->nextFCB = fcbout;
	;

	//STDERR
	struct FCB *fcberr = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcberr->fileDescriptor = STDERR;
	fcberr->deviceType = CONS;
	fcbout->nextFCB = fcberr;
	fcberr->nextFCB = 0;

	// Run the forked process
	asm("cli");
	LinkTask(task);
	asm("sti");

	// We want the forked process to return to this point. So we
	// need to save the registers from here to the new task structure.
	SaveRegisters(task);

	// Return 0 to the forked process, the new pid to the forking one.
	if (pid == currentTask->pid)
		pid = 0;
	return pid;
}

//===============================================================================
// This loads the program "name" into memory, if it exists.
//===============================================================================
long DoExec(char *name, char *environment)
{
	struct FCB *fHandle;
	long codelen, datalen;
	char header[15];
	long *data;
	long currentPage;
	long size;
	long argc;
	long argv;

	char *kname = AllocKMem(strlen(name) + 6); // Enough space for "/BIN" + name

	strcpy(kname, "/BIN/");
	strcat(kname, name);

	struct Message *FSMsg = ALLOCMSG;

	// Open file
	FSMsg->nextMessage = 0;
	FSMsg->byte = OPENFILE;
	FSMsg->quad = (long) kname;
	FSMsg->quad2 = (long) fHandle;
	SendReceiveMessage(FSPort, FSMsg);

	fHandle = (struct FCB *) FSMsg->quad;
	if (fHandle)
	{
		ReadFromFile(fHandle, header, 14);
		ReadFromFile(fHandle, (char *) &codelen, 8);
		ReadFromFile(fHandle, (char *) &datalen, 8);

		// Allocate pages for user code
		currentPage = UserCode;
		size = codelen;
		ClearUserMemory();
		while (size > 0)
		{
			AllocAndCreatePTE(currentPage, currentTask->pid, RW | US | P);
			size -= PageSize;
			currentPage += PageSize;
		}
		copyMem((char *) header, (char *) UserCode, 14);
		copyMem((char *) &codelen, (char *) UserCode + 14, 8);
		copyMem((char *) &datalen, (char *) UserCode + 22, 8);
		char *location = (char *) UserCode + 30;

		// Load the user code
		ReadFromFile(fHandle, location, codelen - 30);

		// Allocate pages for user data
		currentPage = UserData;
		size = datalen + sizeof(struct MemStruct);
		while (size > 0)
		{
			AllocAndCreatePTE(currentPage, currentTask->pid, RW | US | P);
			size -= PageSize;
			currentPage += PageSize;
		}

		// Load the user data
		int bytesRead = ReadFromFile(fHandle, (char *) UserData, datalen);
		Debug();

		// Zero the rest of the data segment
		char *temp = UserData + bytesRead;
		while (bytesRead >= PageSize)
			bytesRead -= PageSize;
		int n;
		for (n = 0; n < bytesRead; n++)
			temp[n] = 0;

		currentTask->firstfreemem = UserData + datalen;

		//Close file and deallocate memory for structures
		FSMsg->nextMessage = 0;
		FSMsg->byte = CLOSEFILE;
		FSMsg->quad = (long) fHandle;
		SendReceiveMessage(FSPort, FSMsg);
		DeallocMem(kname);
		DeallocMem(FSMsg);

		// Process the arguments for argc and argv
		// Copy environment string to user data space
		copyMem(environment, (void *) (UserData + datalen), 81);
		currentTask->firstfreemem += 81;
		currentTask->environment = (void *) UserData + datalen;
		long argv = (long) currentTask->environment;
		argc = ParseEnvironmentString(&argv);
		argv += 80;
		currentTask->firstfreemem += argc * sizeof(char *);
		long *l = (long *) (UserCode + 22);
		*l = (long) (currentTask->firstfreemem) - UserData;
		asm("mov %0,%%rdi;" "mov %1,%%rsi":
				: "r"(argc), "r"(argv):"%rax", "%rdi");
		return 0;
	}
	else
	{
		DeallocMem(kname);
		DeallocMem(FSMsg);
		return 1;
	}
}

//==================================================================
// Set current task to wait for process pid to end
///=================================================================
void Do_Wait(unsigned short pid)
{
	struct Task *task = PidToTask(pid);
	struct MessagePort *parentPort = AllocMessagePort();
	struct Message *message = ALLOCMSG;

	task->parentPort = parentPort;
	ReceiveMessage(parentPort, message);
	DeallocMem(message);
	DeallocMem(parentPort);
}

//==========================
// Create a new Kernel task
//==========================
struct Task * NewKernelTask(void *TaskCode)
{
	long *stack;
	struct Task *task = (struct Task *) AllocKMem(sizeof(struct Task));
	long *data;

	task->pid = nextpid++;
	task->waiting = 0;
	task->cr3 = (long) VCreatePageDir(task->pid, 0);
	task->ds = data64;
	stack = (long *) (TempUStack + PageSize) - 5;
	task->rsp = (long) ((long *) (UserStack + PageSize) - 5);
	task->r15 = (long) task;
	stack[0] = (long) TaskCode;
	stack[1] = code64;
	stack[2] = 0x2202;
	stack[3] = (long) UserStack + PageSize;
	stack[4] = data64;
	asm("cli");
	LinkTask(task);
	data = (long *) TempUserData;
	data[0] = 0;
	data[1] = PageSize - sizeof(struct MemStruct);
	task->firstfreemem = UserData;
	task->environment = (void *) 0;
	task->parentPort = (void *) 0;
	task->currentDirName = currentTask->currentDirName;
	task->argv = 0;
	task->console = 0;
	asm("sti");
	return (task);
}

//=============================
// Create the low-priority task
//=============================
void NewLowPriTask(void *TaskCode)
{
	lowPriTask = NewKernelTask(TaskCode);
	runnableTasks = RemoveFromTaskList(runnableTasks, lowPriTask);
	lowPriTask->nexttask = (struct Task *) 0;
}

//=======================
// Kill the current task
//=======================
void KillTask(void)
{
	struct Task *task = currentTask;

	// Is there a task waiting for this one to finish? Send it a message.
	if (task->parentPort)
	{
		struct Message *m = ALLOCMSG;
		m->quad = 0;
		SendMessage(task->parentPort, m);
		DeallocMem(m);
	}

	if (task->environment)
		DeallocMem(task->environment);
	if (task->argv)
		DeallocMem(task->argv);

	//Don't want to task switch whilst destroying task
	asm("cli");

	// Unlink task from runnable queue
	runnableTasks = RemoveFromTaskList(runnableTasks, task);

	// Deallocate FCBs
	struct FCB *temp;
	while (task->fcbList)
	{
		temp = task->fcbList->nextFCB;
		if (task->fcbList->deviceType == FILE)
			DoClose(task->fcbList);
		else
			DeallocMem(task->fcbList);
		task->fcbList = temp;
	}

	// Deallocate currentDirName - bit of a kludge here!!!
	if (currentTask->pid != 2)
		DeallocMem(currentTask->currentDirName);

	// Add the task to the Dead Tasks queue
	deadTasks = AddToHeadOfTaskList(deadTasks, currentTask);

	// Remove task from allTasks queue;
	allTasks = RemoveFromTaskList(allTasks, currentTask);

//	task->pid = 0;
	SWTASKS;
}

//===============================================
// Move task from runnable queue to blocked queue
//===============================================
void BlockTask(struct Task *task)
{
	canSwitch++;
	runnableTasks = RemoveFromTaskList(runnableTasks, task);
	blockedTasks = AddToTailOfTaskList(blockedTasks, task);
	canSwitch--;
}

//===============================================
// Move task from blocked queue to runnable queue
//===============================================
void UnBlockTask(struct Task *task)
{
	canSwitch++;
	blockedTasks = RemoveFromTaskList(blockedTasks, task);
	runnableTasks = AddToTailOfTaskList(runnableTasks, task);
	canSwitch--;
}

//=========================================
// Returns the task structure with PID pid
//=========================================
struct Task * PidToTask(long pid)
{
	struct TaskList *tempTask = allTasks;

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
void dummyTask()
{
	unsigned short int pid;
	struct TaskList *t;
	long count;

	while (1)
	{
		if (deadTasks)
		{
			t = deadTasks;
			deadTasks = deadTasks->next;
			pid = t->task->pid;
			DeallocMem(t->task);
			DeallocMem(t);
			for (count = 0; count < nPages; count++)
			{
				if (PMap[count] == pid)
				{
					PMap[count] = 0;
					ZeroPage(count);
					if (count < firstFreePage)
						firstFreePage = count;
					nPagesFree++;
				}
			}
#ifdef DEBUG
			int free = 0;
			for (count = 0; count <nPages; count++)
			if (!PMap[count]) free++;
			kprintf(24, 0, "%d %d", free, nPagesFree);
#endif
		}
		else
			asm("hlt");
	}
}

//======================================================================
// Parses argv[][] from the environment string.
// Returns argc.
//======================================================================
long ParseEnvironmentString(long *l)
{
	long argc = 0;
	int count = 0;

	char *env = currentTask->environment;

	long *argv = (long *) (env + 80);
	argv[0] = (long) env;
	while (env[count])
	{
		while (env[count] && env[count] != ' ')
			count++;
		argc++;
		if (env[count])
		{
			env[count] = 0;
			argv[argc] = (long) env + ++count;
		}
	}
	return argc;
}

extern void kbTaskCode(void);
extern void consoleTaskCode(void);
extern void fsTaskCode(void);

//===================================================================
// This starts the dummy task and the services
// Any new service will need to be added to this list
// Sleep after each task to give it time to initialize itself
// I'm hoping this will improve system stability
//===================================================================
void StartTasks()
{
	NewLowPriTask(dummyTask);
	GoToSleep(10);
	NewKernelTask(kbTaskCode);
	GoToSleep(10);
	NewKernelTask(consoleTaskCode);
	GoToSleep(10);
	NewKernelTask(fsTaskCode);
	GoToSleep(10);
}
