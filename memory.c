#include <kernel.h>

extern struct Task *currentTask;
extern struct TaskList *runnableTasks;
extern struct TaskList *blockedTasks;
extern struct TaskList *allTasks;
extern struct Task *lowPriTask;
extern struct TaskList *deadTasks;
extern long canSwitch;
extern long pass;
extern long nextpid;
unsigned char *oMemMax;
long nPagesFree;
long nPages;
long firstFreePage;
struct MemStruct *firstFreeKMem;
long nextKPage;
unsigned short int *PMap;
long memorySemaphore;
long initialCR3;

long sec, min, hour, day, month, year, tenths, unixtime;

struct MessagePort *KbdPort;
struct MessagePort *ConsolePort;
struct MessagePort *FSPort;

void InitMem64(void)
{
	PMap = (unsigned short int *) PageMap;
	firstFreeKMem = (struct MemStruct *) OSHeap;
	firstFreeKMem->size = PageSize - sizeof(struct MemStruct);
	currentTask = (struct Task *) AllocKMem(sizeof(struct Task));
	nextKPage = 0x12;
	nextpid = 3;
	runnableTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	runnableTasks->next = 0L;
	runnableTasks->task = currentTask;
	allTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	allTasks->task = currentTask;
	allTasks->next = 0;
	deadTasks = 0;
	lowPriTask = 0L;
	blockedTasks = 0L;
	memorySemaphore = 0;
	canSwitch = 0;
	tenths = 0;
	pass = 0;
}

//=========================================================================================
// Searches the linked list pointed to by list for a block of memory of size sizeRequested
// Allocates the memory and returns its address in RAX
//=========================================================================================
void * AllocMem(long sizeRequested, struct MemStruct *list)
{
	unsigned char kernel = 0;
	if (list == firstFreeKMem)
		kernel = 1;
	// We want the memory allocation to be atomic, so set a semaphore before proceeding
	SetSem(&memorySemaphore);

	while (list->next)
	{
		if (list->size >= sizeRequested)
			break;
		list = list->next;
	}

	if (!list->next)
	{
		// End of list. Enough memory available? If not allocate new pages until there is.
		while (list->size <= sizeRequested + sizeof(struct MemStruct))
		{
			long temp = (long) list >> 12;
			long temp2 = list->size;
			while (temp2 > PageSize)	// There may already be allocated pages beyond the current one!
			{
				temp++;
				temp2 -= PageSize;
			}
			while (list->size < sizeRequested + 2 * sizeof(struct MemStruct))
			{
				if (kernel)
					AllocAndCreatePTE(++temp << 12, 1, RW | G | P);
				else
					AllocAndCreatePTE(++temp << 12, currentTask->pid,
							RW | US | P);
				list->size += PageSize;
			}
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
		list->next->size = list->size - sizeRequested
				- sizeof(struct MemStruct);
		list->size = 0;
	}
	ClearSem(&memorySemaphore);
	return (list + 1);
}

//==================================================
// Deallocate the memory at location list.
// This will deallocate both user and kernel memory
//==================================================
void DeallocMem(void *list)
{
	struct MemStruct *l = (struct MemStruct *) list;

	// We want the memory deallocation to be atomic, so set a semaphore before proceeding
	SetSem(&memorySemaphore);
	l--;
	if (!l->size)
	{
		l->size = (long) l->next - (long) l - sizeof(struct MemStruct);
	}
	ClearSem(&memorySemaphore);
}

//===============================================================================
// Allocate some kernel memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *AllocKMem(long sizeRequested)
{
	return (AllocMem(sizeRequested, firstFreeKMem));
}

//===============================================================================
// Allocate some user memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *AllocUMem(long sizeRequested)
{
	return (AllocMem(sizeRequested, (void *) currentTask->firstfreemem));
}
