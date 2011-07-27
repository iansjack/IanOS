#include "memory.h"
#include "tasklist.h"

struct TaskList * AddToHeadOfTaskList(struct TaskList * list, struct Task * task)
{
	struct TaskList * temp = (struct TaskList *)AllocKMem(sizeof (struct TaskList));
	temp->task = task;
	temp->next = list;
	return temp;
}

struct TaskList * AddToTailOfTaskList(struct TaskList * list, struct Task * task)
{
	struct TaskList * temp = (struct TaskList *)AllocKMem(sizeof (struct TaskList));
	struct TaskList  * start = list;

	temp->next = 0;
	temp->task = task;
	if (list == 0)
	{
		list = temp;
		return temp;
	}
	else
	{
		while (list->next) list = list->next;
		list->next = temp;
		return start;
	}

}

struct TaskList * RemoveFromTaskList(struct TaskList * list, struct Task * task)
{
	struct TaskList *start = list;

	   if (list->task == task)
	   {
	      list = list->next;
	      DeallocMem(start);
	      return list;
	   }
	   else
	   {
	      while (list->next->task != task)
	      {
	         list = list->next;
	      }
	      struct TaskList * temp = list->next;
	      list->next = list->next->next;
	      DeallocMem(temp);
	      return start;
	   }
}

//===========================================================
// Move the task at the head of the runnable queue to the end
//===========================================================
struct TaskList * MoveTaskToEndOfList(struct TaskList * list)
{
	if (!list) return list;
	if (!list->next) return list;

	struct TaskList * start = list;
	struct TaskList * temp = list;
	list = list->next;
	while (temp->next) temp = temp->next;
	temp->next = start;
	start->next = 0;
	return list;
}
