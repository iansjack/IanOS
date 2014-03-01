#include <kernel.h>
#include <pagetab.h>

// #define NULL 0;

extern struct Task *currentTask;
extern struct TaskList *runnableTasks;
extern struct TaskList *blockedTasks;
extern struct TaskList *allTasks;
extern struct Task *lowPriTask;
extern struct TaskList *deadTasks;
extern long canSwitch;
extern long pass;
extern unsigned short nextpid;
unsigned char *oMemMax;
long nPagesFree;
long nPages;
long firstFreePage;
struct MemStruct *firstFreeKMem;
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
	firstFreeKMem->next = 0;
	firstFreeKMem->size = (long)(PageSize - sizeof(struct MemStruct));
	currentTask = (struct Task *) AllocKMem(sizeof(struct Task));
	nextpid = 3;
	runnableTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	runnableTasks->next = NULL;
	runnableTasks->task = currentTask;
	allTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	allTasks->task = currentTask;
	allTasks->next = 0;
	deadTasks = 0;
	lowPriTask = NULL; //0L;
	blockedTasks = NULL; //0L;
	memorySemaphore = 0;
	canSwitch = 0;
	tenths = 0;
	pass = 0;
}

//=========================================================================================
// Searches the linked list pointed to by list for a block of memory of size sizeRequested
// Allocates the memory and returns its address in RAX. If necessary, maps new pages
// into the heap.
//=========================================================================================
void * AllocMem(size_t sizeRequested, struct MemStruct *list)
{
	unsigned char kernel = 0;
	if (list == firstFreeKMem) kernel = 1;
	// We want the memory allocation to be atomic, so set a semaphore before proceeding
	SetSem(&memorySemaphore);

	while (list->next)
	{
		if (list->size >= (int) sizeRequested) break;
		list = list->next;
	}

	// We have either found a large enough space or we are at the end of the list
	// Test first to see if it's the end of the list
	if (!list->next)
	{
		// End of list. Enough memory available? If not allocate new pages until there is.
		// We need at least the size requested plus space for a new MemStruct record
		while ((size_t) list->size < sizeRequested + sizeof(struct MemStruct))
		{
			// Not enough free space, so allocate another page.
			// Find first unmapped address
			long temp = (long) list + sizeof(struct MemStruct) + list->size;
			if (kernel)
				(void) AllocAndCreatePTE(temp, 1, RW | G | P);
			else
				(void) AllocAndCreatePTE(temp, currentTask->pid, RW | US | P);
			list->size += PageSize;
		}
	}

	// We now have found a free memory block with enough (or more space)
	// Is there enough space for another link?
	if ((size_t) list->size < sizeRequested + sizeof(struct MemStruct))
	{
		// No. Just allocate the whole block
		// Was that the last entry in the list?
		if (!list->next)
		{
			void *temp = (void *)list + sizeof(struct MemStruct) + sizeRequested;
			((struct MemStruct *)temp)->next = 0;
			((struct MemStruct *)temp)->size = list->size - sizeRequested - sizeof(struct MemStruct);
			list->next = temp;
		}
		// Allocate the block
		list->size = 0;
	}
	else
	{
		// Yes, so create link to the new free block
		void *temp = (void *) list + sizeof(struct MemStruct) + sizeRequested;
		((struct MemStruct *) temp)->next = list->next;
		((struct MemStruct *) temp)->size = list->size - sizeRequested - sizeof(struct MemStruct);

		// link the new free block into the list
		list->next = (struct MemStruct *) temp;

		// Allocate the block
		list->size = 0;
	}
	ClearSem(&memorySemaphore);

	// Zero fill the allocated memory
	memset((char *)(list + 1), 0, sizeRequested);

	// Return a pointer to the memory (i.e. skip the struct)
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
void *AllocKMem(size_t sizeRequested)
{
	return (AllocMem(sizeRequested, firstFreeKMem));
}

//===============================================================================
// Allocate some user memory from the heap. sizeRequested = amount to allocate
// Returns in RAX address of allocated memory.
//===============================================================================
void *AllocUMem(size_t sizeRequested)
{
	return (AllocMem(sizeRequested, (void *) currentTask->firstfreemem));
}
