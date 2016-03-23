#include <linux/types.h>
#include <kernel.h>
#include <filesystem.h>
#include <errno.h>
#include <elf.h>
#include <blocks.h>
#include <pagetab.h>
#include <elffunctions.h>
#include <reent.h>

extern void *registers;

void GoToSleep(long);	// Defined in syscalls.s

void SaveRegisters(struct Task *);	// Defined in tasking.s
struct Task *NewKernelTask(void *TaskCode);
long ParseEnvironmentString(long *);

struct Task *currentTask;
struct TaskList *runnableTasks;
struct TaskList *blockedTasks;
struct Task *lowPriTask;
struct TaskList *allTasks;
struct TaskList *deadTasks;
long canSwitch;
long pass;
void *entrypoint;

extern long nPagesFree;
extern struct MessagePort *FSPort;

unsigned short nextpid;

//===============================
// Link task into the task table
//===============================
void LinkTask(struct Task *task)
{
	asm("pushf");
	asm("cli");
	allTasks = AddToTailOfTaskList(allTasks, task);
	runnableTasks = AddToTailOfTaskList(runnableTasks, task);
	asm("popf");
}

//================================================================
// Copy the given page and all consecutive subsequent mapped ones
//================================================================
void CopyPages(l_Address address, struct Task *task)
{
	// Page align the address
	address = PAGE(address);

	// Get physical address of page tables
	struct PML4 *pml4 = (struct PML4 *) (task->cr3);
	struct PML4 *current_pml4 = (struct PML4 *) (currentTask->cr3);

	while (1)
	{
		struct PT *pt = GetPT(pml4, address, task->pid);
		struct PT *currentPT = GetPT(current_pml4, address, currentTask->pid);

		unsigned int i = GetPTIndex(address);
		if (!(VIRT(PT,currentPT) ->entries[i].value))
			break;

		// Create and map the new page and copy the physical memory
		if (!checkPTEWithPT(pml4, address))
		{
			p_Address data = AllocPage(task->pid);
			CreatePTEWithPT((struct PML4 *) task->cr3, data, address, task->pid,
					US | RW | P | 0x800);
		}
		memcpy((void *) PAGE(((VIRT(PT, pt)) ->entries[i].value)) + VAddr,
				(void *) PAGE(((VIRT(PT, currentPT)) ->entries[i].value))
						+ VAddr, PageSize);
		address += PageSize;
	}

}

extern long currAlloc;
extern struct library *libs;

//=========================================================================
// Fork the current process.
// Return the pid of the new process
//=========================================================================
unsigned short DoFork()
{
//	currAlloc = 0;

	unsigned short pid;
	struct FCB *fcbin, *fcbout, *fcberr;

	// Copy task structure, with adjustments
	struct Task *task = (struct Task *) AllocKMem(sizeof(struct Task));

	memcpy((char *) task, (char *) currentTask, sizeof(struct Task));

	pid = task->pid = nextpid++;
	task->currentDirName = AllocKMem(
			(size_t) strlen(currentTask->currentDirName) + 1);
	strcpy(task->currentDirName, currentTask->currentDirName);
	task->parentPort = 0;

	// Create Page Directory
	task->cr3 = (long) VCreatePageDir(pid, currentTask->pid);

	Elf64_Ehdr *header = (Elf64_Ehdr *) UserCode;
	if ((header->e_ident)[1] != 'E')// Not the best of tests, but good enough
		CopyPages(UserCode, task);
	else
	{
		Elf64_Phdr *pheadertable = (Elf64_Phdr *) (UserCode + header->e_phoff);
		int i;

		for (i = 0; i < header->e_phnum; i++)
		{
			if (pheadertable[i].p_type == PT_LOAD)
			{
				CopyPages(pheadertable[i].p_vaddr, task);
			}
		}
		struct library *lib = libs;
		while (lib)
		{
			CopyPages(lib->base, task);
			CopyPages(lib->data_base, task);
			lib = lib->next;
		}
	}

	// Copy user stack and kernel stack
	CopyPages(UserStack, task);

	// Copy data area
	CopyPages(UserData, task);

	//Page Tables to allow access to E1000
	CreatePTEWithPT((struct PML4 *) task->cr3, registers, (long) registers, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x1000,
			(long) registers + 0x1000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x2000,
			(long) registers + 0x2000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x3000,
			(long) registers + 0x3000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x4000,
			(long) registers + 0x4000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x5000,
			(long) registers + 0x5000, 0, 7);
	task->forking = 1;

	// Create FCBs for STDI, STDOUT, and STDERR

	// STDIN
	fcbin = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcbin->fileDescriptor = STDIN;
	fcbin->deviceType = KBD;
	task->fcbList = fcbin;

	// STDOUT
	fcbout = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcbout->fileDescriptor = STDOUT;
	fcbout->deviceType = CONS;
	fcbin->nextFCB = fcbout;

	//STDERR
	fcberr = (struct FCB *) AllocKMem(sizeof(struct FCB));
	fcberr->fileDescriptor = STDERR;
	fcberr->deviceType = CONS;
	fcbout->nextFCB = fcberr;
	fcberr->nextFCB = 0;
	task->FDbitmap = 7;

	// Run the forked process
	asm("pushf");
	asm("cli");
	LinkTask(task);
	asm("popf");

	// We want the forked process to return to this point. So we
	// need to save the registers from here to the new task structure.
	SaveRegisters(task);

	// Return 0 to the forked process, the new pid to the forking one.
	if (pid == currentTask->pid)
		pid = 0;
	return pid;
}

//======================================
// Load a flat format binary executable
//======================================
void LoadFlat(struct FCB * fHandle)
{
	char header[15];
	char *location, *temp;
	long codelen, datalen, currentPage, size;
	int bytesRead, n;

	(void) ReadFromFile(fHandle, header, 14);
	(void) ReadFromFile(fHandle, (char *) &codelen, 8);
	(void) ReadFromFile(fHandle, (char *) &datalen, 8);

	// Allocate pages for user code
	currentPage = UserCode;
	size = codelen;
	ClearUserMemory();
	while (size > 0)
	{
		(void) AllocAndCreatePTE(currentPage, currentTask->pid, RW | US | P);
		size -= PageSize;
		currentPage += PageSize;
	}
	memcpy((char *) UserCode, (char *) header, 14);
	memcpy((char *) UserCode + 14, (char *) &codelen, 8);
	memcpy((char *) UserCode + 22, (char *) &datalen, 8);
	location = (char *) UserCode + 30;

	// Load the user code
	(void) ReadFromFile(fHandle, location, codelen - 30);
	entrypoint = (void *) UserCode;

	// Allocate pages for user data
	currentPage = UserData;
	size = datalen + sizeof(struct MemStruct);
	while (size > 0)
	{
		(void) AllocAndCreatePTE(currentPage, currentTask->pid, RW | US | P);
		size -= PageSize;
		currentPage += PageSize;
	}

	// Load the user data
	bytesRead = ReadFromFile(fHandle, (char *) UserData, datalen);

	// Zero the rest of the data segment
	temp = (char *) (UserData + bytesRead);
	while (bytesRead >= PageSize)
		bytesRead -= PageSize;
	for (n = 0; n < bytesRead; n++)
		temp[n] = 0;

	currentTask->firstfreemem = UserData + datalen;
}

//======================================
// Load an ELF format binary executable
//======================================
void LoadElf(struct Message *FSMsg, struct FCB * fHandle)
{
//	ClearUserMemory();
	ReadElf(FSMsg, fHandle, 0, &entrypoint, currentTask->pid);
//	(void) AllocAndCreatePTE(UserData, currentTask->pid, RW | US | P);
//	currentTask->firstfreemem = UserData;
}

//===============================================================================
// This loads the program "name" into memory, if it exists.
//===============================================================================
long DoExec(char *name, char *environment)
{
	struct FCB *fHandle;
	long argv, argc;
	int retval = -ENOEXEC;
	struct Message *FSMsg = ALLOCMSG;

	char *kname = AllocKMem((size_t) strlen(name) + 6); // Enough space for "/bin" + name

	strcpy(kname, "/bin/");
	strcat(kname, name);

	// Open file
	FSMsg->nextMessage = 0;
	FSMsg->byte = OPENFILE;
	FSMsg->quad1 = (long) kname;
	FSMsg->quad2 = (long) fHandle;
	SendReceiveMessage(FSPort, FSMsg);

	fHandle = (struct FCB *) FSMsg->quad1;

	if ((long) fHandle > 0)
	{
		char magic[5];
		char executable = 0;
		(void) SeekFile(FSMsg, fHandle, 10, SEEK_SET);
		(void) ReadFromFile(fHandle, magic, 4);
		magic[4] = 0;
		if (!strcmp(magic, "IJ64"))
		{
			(void) SeekFile(FSMsg, fHandle, 0, SEEK_SET);
			LoadFlat(fHandle);
			executable = 1;
		}
		else
		{
			(void) SeekFile(FSMsg, fHandle, 0, SEEK_SET);
			(void) ReadFromFile(fHandle, magic, 4);
			if (magic[0] == 0x7F)
			{
				(void) SeekFile(FSMsg, fHandle, 0, SEEK_SET);
				LoadElf(FSMsg, fHandle);
				executable = 1;
			}
		}

		//Close file and deallocate memory for structures
		FSMsg->nextMessage = 0;
		FSMsg->byte = CLOSEFILE;
		FSMsg->quad1 = (long) fHandle;
		SendReceiveMessage(FSPort, FSMsg);
		DeallocMem(kname);
		DeallocMem(FSMsg);

		if (executable)
		{
			long *l;

			// Process the arguments for argc and argv
			// Copy environment string to user data space
			// It occupies the 81 bytes after the current first free memory
			currentTask->environment = UserData; //(void *) currentTask->firstfreemem;
			memcpy(currentTask->environment, environment, 80);
			currentTask->firstfreemem = UserData + PageSize; //+= 80;
			argv = (long) currentTask->environment;
			argc = ParseEnvironmentString(&argv);
			argv += 80;

			// Adjust firstfreemem to point to the first free memory location.
			//currentTask->firstfreemem += argc * sizeof(char *);

			// Build the first MemStruct struct. Is all this necessary? User tasks don't use the kernel memory allocation, do they?
			//l = (long *) (currentTask->firstfreemem);
			//*l = 0;
			//*(l + 1) = -(long) (((sizeof(struct MemStruct)
			//		+ currentTask->firstfreemem)) % PageSize);
			asm("mov %0,%%rdi;" "mov %1,%%rsi":
					: "r"(argc), "r"(argv):"%rax", "%rdi");
			return 0;
		}
		else
			return retval;
	}
	else
	{
		DeallocMem(kname);
		DeallocMem(FSMsg);
		return -ENOENT;
	}
}

//==================================================================
// Block the current task to wait for process pid to end
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
struct Task *NewKernelTask(void *TaskCode)
{
//	asm("jmp .");
	long *stack;
	struct Task *task = (struct Task *) AllocKMem(sizeof(struct Task));
	long *data;

	task->pid = nextpid++;
	task->waiting = 0;
	task->cr3 = (long) VCreatePageDir(task->pid, 0);

	//Page Tables to allow access to E1000
	CreatePTEWithPT((struct PML4 *) task->cr3, registers, (long) registers, 0,
			7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x1000,
			(long) registers + 0x1000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x2000,
			(long) registers + 0x2000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x3000,
			(long) registers + 0x3000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x4000,
			(long) registers + 0x4000, 0, 7);
	CreatePTEWithPT((struct PML4 *) task->cr3, registers + 0x5000,
			(long) registers + 0x5000, 0, 7);


	task->ds = data64;
	stack = (long *) AllocPage(task->pid);
	CreatePTEWithPT((struct PML4 *) task->cr3, (p_Address) stack, UserStack,
			task->pid, RW | P);
	stack += VAddr / 8;
	stack += PageSize / 8;
	stack -= 5;
	task->rsp = (long) ((long *) (UserStack + PageSize) - 5);
	task->r15 = (long) task;
	stack[0] = (long) TaskCode;
	stack[1] = code64;
	stack[2] = 0x2202;
	stack[3] = (long) UserStack + PageSize;
	stack[4] = data64;
	asm("pushf");
	asm("cli");
	LinkTask(task);
	data = (long *) AllocPage(task->pid);
	CreatePTEWithPT((struct PML4 *) task->cr3, (p_Address) data, UserData,
			task->pid, RW | P);
	data += VAddr / 8;
	data[0] = 0;
	data[1] = (long) (PageSize - sizeof(struct MemStruct));
	task->firstfreemem = UserData;
	task->environment = (void *) 0;
	task->parentPort = (void *) 0;
	task->currentDirName = currentTask->currentDirName;
	task->argv = 0;
	task->console = 0;
	asm("popf");
	return (task);
}

//=============================
// Create the low-priority task
//=============================
void NewLowPriTask(void *TaskCode)
{
	lowPriTask = NewKernelTask(TaskCode);
	runnableTasks = RemoveFromTaskList(runnableTasks, lowPriTask);
}

//=======================
// Kill the current task
//=======================
void KillTask(void)
{
	struct Task *task = currentTask;
	struct FCB *temp;

	// Is there a task waiting for this one to finish? Send it a message.
	if (task->parentPort)
	{
		struct Message *m = ALLOCMSG;
		m->quad1 = 0;
		SendMessage(task->parentPort, m);
		DeallocMem(m);
	}

	/*if (task->environment)
	 DeallocMem(task->environment);
	 if (task->argv)
	 DeallocMem(task->argv);
	 */

	//Don't want to task switch whilst destroying task
	// asm("cli");
	// Unlink task from runnable queue
	runnableTasks = RemoveFromTaskList(runnableTasks, task);

	// Deallocate FCBs
	while (task->fcbList)
	{
		temp = task->fcbList->nextFCB;
		if (task->fcbList->deviceType == ORDINARY_FILE)
			DoClose(task->fcbList->fileDescriptor);
		else
			DeallocMem(task->fcbList);
		task->fcbList = temp;
	}

	// Deallocate currentDirName - bit of a kludge here!!!
	//if (currentTask->pid != 2)
	if (currentTask->currentDirName)
		DeallocMem(currentTask->currentDirName);

	// Remove task from allTasks queue;
	allTasks = RemoveFromTaskList(allTasks, currentTask);

	// Add the task to the Dead Tasks queue
	deadTasks = AddToHeadOfTaskList(deadTasks, currentTask);

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
struct Task *
PidToTask(unsigned short pid)
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

//================================
// Return a page to the free pool
//================================
void ClearIfUser(p_Address n)
{
	if (n & 0x800)
	{
		ClearBit(n >> 12);
		nPagesFree++;
	}
}

//=====================================
// The one task that is always runnable
//=====================================
void dummyTask()
{
	unsigned short int pid;
	struct TaskList *t;
	unsigned long cr3;
	unsigned long i, j, k, l;

	while (1)
	{
		if (deadTasks)
		{
			t = deadTasks;
			deadTasks = deadTasks->next;

			// Release user pages
			struct PML4 *pml4 = (struct PML4 *) (PAGE(t->task->cr3));
			for (i = 0; i < 512; i++)
			{
				unsigned long temp = (VIRT(PML4,pml4) ->entries[i].value);
				if (temp)
				{
					for (j = 0; j < 512; j++)
					{
						unsigned long temp2 =
								(VIRT(PDP,(PAGE(temp))) ->entries[j].value);
						if (temp2)
						{
							for (k = 0; k < 512; k++)
							{
								unsigned long temp3 =
										(VIRT(PD,(PAGE(temp2))) ->entries[k].value);
								if (temp3)
								{
									for (l = 0; l < 512; l++)
									{
										unsigned long temp4 =
												(VIRT(PT,(PAGE(temp3))) ->entries[l].value);
										if (temp4)
											ClearIfUser(temp4);
									}
									ClearIfUser(temp3);
								}
							}
							ClearIfUser(temp2);
						}
					}
					ClearIfUser(temp);
				}
			}
			ClearBit((long) pml4 >> 12);
			nPagesFree++;

			DeallocMem(t->task);
			DeallocMem(t);
		}
		else
			asm("hlt");
		kprintf(0, 60, "Free Pages = %d", nPagesFree);
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
	kprintf(0, 0, "Starting tasks");
	(void) NewLowPriTask((void *) dummyTask);
	GoToSleep(100);
	(void) NewKernelTask((void *) kbTaskCode);
	GoToSleep(100);
	(void) NewKernelTask((void *) consoleTaskCode);
	GoToSleep(100);
	(void) NewKernelTask((void *) fsTaskCode);
	GoToSleep(100);
}
