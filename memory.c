#include "memory.h"
#include "kernel.h"
#include "tasklist.h"

//#define DEBUG

#ifdef DEBUG
struct MemoryAllocation
{
	void *memory;
	long size;
	void *allocated;
	void *deallocated;
};
#endif

extern struct Task *currentTask;
extern struct TaskList *runnableTasks;
extern struct TaskList *blockedTasks;
extern struct TaskList *allTasks;
extern struct Task *lowPriTask;
extern struct TaskList *deadTasks;
extern long canSwitch;
extern long pass;

unsigned char *oMemMax;
long nPagesFree;
long nPages;
struct MemStruct *firstFreeKMem;
/*static*/
long nextKPage;
unsigned short int *PMap;
long memorySemaphore;

#ifdef DEBUG
long NoOfAllocations;
struct MemoryAllocation allocations[32];
int count;
#endif
long debugging;

void InitMem64(void)
{
	PMap = (unsigned short int *) PageMap;
	firstFreeKMem = (struct MemStruct *) 0x11000;
	firstFreeKMem->next = 0;
	firstFreeKMem->size = 0xFE0;
	nextKPage = 0x12;
	currentTask = (struct Task *) TaskStruct;
	runnableTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	runnableTasks->next = 0L;
	runnableTasks->task = currentTask;
	allTasks = (struct TaskList *) AllocKMem(sizeof(struct TaskList));
	allTasks->task = currentTask;
	allTasks->next = 0;
	deadTasks = 0;
	lowPriTask = 0L;
	blockedTasks = 0L;

#ifdef DEBUG
	NoOfAllocations = 0;
	for (count = 0; count < 32; count++)
	{
		allocations[count].memory =
		allocations[count].size =
		allocations[count].allocated =
		allocations[count].deallocated = 0;
	}
	count = 0;
	debugging = 0;
#endif
	memorySemaphore = 0;
	canSwitch = 0;
	pass = 0;
}

//=========================================================================================
// Searches the linked list pointed to by list for a block of memory of size sizeRequested
// Allocates the memory and returns its address in RAX
//=========================================================================================
void *AllocMem(long sizeRequested, struct MemStruct *list)
{
	unsigned char kernel = 0;
	if (list == firstFreeKMem)
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
					AllocAndCreatePTE(++temp << 12, 1, RW | G | P);
				else
					AllocAndCreatePTE(++temp << 12, currentTask->pid, RW | US | P);
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
		list->next->size = list->size - sizeRequested
				- sizeof(struct MemStruct);
		list->size = 0;
		list->pid = currentTask->pid;
	}
	ClearSem(&memorySemaphore);
#ifdef DEBUG
	NoOfAllocations++;
	KWriteHex(NoOfAllocations, 24);
	if (debugging == 1)
	{
		allocations[count].size = sizeRequested;
		allocations[count].memory = list + 1;
		allocations[count].allocated = 1;
		count++;
		if (count == 32)
		debugging = 0;
	}
#endif
	return (list + 1);
}

//==================================================
// Deallocate the memory at location list.
// This will deallocate both user and kernel memory
//==================================================
void DeallocMem(void *list)
{
	if (list)
	{
		struct MemStruct *l = (struct MemStruct *) list;

		// We want the memory deallocation to be atomic, so set a semaphore before proceeding
		SetSem(&memorySemaphore);
		l--;
		if (!l->size)
		{
			l->size = (long) l->next - (long) l - sizeof(struct MemStruct);
#ifdef DEBUG
			NoOfAllocations--;
			KWriteHex(NoOfAllocations, 24);
			if (debugging == 1)
			{
				int i;
				for (i = 0; i < 32; i++)
				if (allocations[i].memory == list &&
						allocations[i].deallocated == 0)
				{
					allocations[i].deallocated = 1;
					break;
				}
			}
#endif
		}
		ClearSem(&memorySemaphore);
	}
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
