#ifndef TASKLIST_H
#define TASKLIST_H

struct TaskList {
	struct TaskList *next;
	struct Task *task;
};

struct TaskList *AddToHeadOfTaskList(struct TaskList *list, struct Task *task);
struct TaskList *AddToTailOfTaskList(struct TaskList *list, struct Task *task);
struct TaskList *RemoveFromTaskList(struct TaskList *list, struct Task *task);
struct TaskList *MoveTaskToEndOfList(struct TaskList *list);

#endif
