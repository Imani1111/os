#ifndef TASKMGR_H
#define TASKMGR_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#include <stddef.h>

#define TASK_STACK_SPACE 4096
#define EFLAGS 0x202

typedef enum {
	TASK_RUNNING,
	TASK_READY,
	TASK_BLOCKED,
	TASK_ZOMBIE
}TaskState;

typedef struct TaskControlBlock {
	struct TaskControlBlock* prev;
	struct TaskControlBlock* next;
	uint32_t esp;
	uint32_t* base;
	int pid;
	char name[32];
	TaskState state;
	int waiting_on_pid;
}TaskControlBlock_t;

extern TaskControlBlock_t* head_task;
extern TaskControlBlock_t* tail_task;
extern TaskControlBlock_t* current_task;

extern int next_pid;

void TaskInit(void);
TaskControlBlock_t* SpawnTask(void(*entry_function)(void), const char* name);
uint32_t Schedule(uint32_t current_esp);
void KillTask(TaskControlBlock_t* task);
void set_task_state(int pid, TaskState state);

#endif
