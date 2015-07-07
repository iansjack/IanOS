#include <memory.h>
#include <tasklist.h>
#include <kernel.h>

extern long canSwitch;
extern struct TaskList *runnableTasks;
extern struct TaskList *blockedTasks;
extern struct TaskList *allTasks;

struct TaskList *AddToHeadOfTaskList(struct TaskList *list, struct Task *task)
{
	struct TaskList *temp;

	canSwitch++;
	if (IsTaskInList(task, list))
	{
		canSwitch--;
		return list;
	}
	temp = (struct TaskList *)AllocKMem(sizeof(struct TaskList));
	temp->task = task;
	temp->next = list;
	canSwitch--;
	return temp;
}

struct TaskList *AddToTailOfTaskList(struct TaskList *list, struct Task *task)
{
	struct TaskList *temp, *start;

	canSwitch++;
	if (IsTaskInList(task, list))
	{
		canSwitch--;
		return list;
	}
	temp = (struct TaskList *)AllocKMem(sizeof(struct TaskList));
	start = list;

	temp->next = 0;
	temp->task = task;
	if (list == 0)
		list = temp;
	else {
		while (list->next)
			list = list->next;
		list->next = temp;
		temp = start;
	}
	canSwitch--;
	return temp;
}

struct TaskList *RemoveFromTaskList(struct TaskList *list, struct Task *task)
{
	struct TaskList *start, *temp;

	canSwitch++;
	if (!IsTaskInList(task, list))
	{
		canSwitch--;
		return list;
	}
	if (list == 0) {
		KWriteString("The list is empty!", 17, 0);
		while (1) ;
	}
	start = list;

	if (list->task == task) {
		list = list->next;
		DeallocMem((void *)start);
		canSwitch--;
		return list;
	} else {
		if (list->next == 0) {
			KWriteString("Null list->next pointer", 18, 0);
			if (start == allTasks)
				KWriteString("Problem with allTasks", 19, 0);
			if (start == runnableTasks)
				KWriteString("Problem with runnableTasks", 19,
					     0);
			if (start == blockedTasks)
				KWriteString("Problem with blockedTasks", 19,
					     0);
			while (1) ;
			canSwitch--;
			return start;
		}

		while (list->next->task != task)
			list = list->next;
		if (list)
		{
			temp = list->next;
			list->next = list->next->next;
			DeallocMem(temp);
		}
		canSwitch--;
		return start;
	}
}

//===========================================================
// Move the task at the head of the runnable queue to the end
//===========================================================
struct TaskList *MoveTaskToEndOfList(struct TaskList *list)
{
	struct TaskList *start, *temp;

	canSwitch++;
	if (!list) {
		canSwitch--;
		return list;
	}
	if (!list->next) {
		canSwitch--;
		return list;
	}

	start = list;
	temp = list;
	list = list->next;
	while (temp->next)
		temp = temp->next;
	temp->next = start;
	start->next = 0;
	canSwitch--;
	return list;
}

//======================================
// Check if a task is in the given list
//======================================
int IsTaskInList(struct Task *task, struct TaskList *list)
{
	while (list)
	{
		if (list->task == task)
			return 1;
		list = list->next;
	}
	return 0;
}
