#include <task_mgr.h>
#include <mem_mgr.h>
#include <screen.h>
#include <idt.h>
#include <string.h>

TaskControlBlock_t* head_task = NULL;
TaskControlBlock_t* tail_task = NULL;
TaskControlBlock_t* current_task = NULL;
int next_pid = 1;

void kernel_idle_loop(){
	while(1){
		asm volatile("hlt");
	}
}


void init_multitasking()
{
	TaskControlBlock_t* kernel_task = (TaskControlBlock_t*)my_malloc(sizeof(TaskControlBlock_t));
	if (kernel_task == NULL){
		print_string_at("Task Init Failed!", 0, 0, 0x00FF0000);
		while (1){
			asm volatile("cli; hlt");
		}
	}
	uint8_t* stack_base = (uint8_t*)my_malloc(TASK_STACK_SPACE);	
	uint32_t* stack_pointer = (uint32_t*)((uint8_t*)stack_base + TASK_STACK_SPACE);
	
	*(--stack_pointer) = EFLAGS;
	*(--stack_pointer) = GDT_CODE_SEGMENT;
	*(--stack_pointer) = (uint32_t)kernel_idle_loop;

	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	
	kernel_task->esp = (uint32_t)stack_pointer;
	kernel_task->base = (uint32_t*)stack_base;
	kernel_task->pid = 0;
	char* name = "Kernel Task";
	kstrcpy(kernel_task->name, name);
	kernel_task->state = TASK_READY;
	kernel_task->waiting_on_pid = -1;
	
	head_task = kernel_task;
	tail_task = kernel_task;
	head_task->next = tail_task;
	head_task->prev = tail_task;
	tail_task->next = head_task;
	tail_task->prev = head_task;
	current_task = kernel_task;
}

TaskControlBlock_t* SpawnTask(void(*entry_point)(void), const char* name)
{
	TaskControlBlock_t* new_task = (TaskControlBlock_t*)my_malloc(sizeof(TaskControlBlock_t));
	if (new_task == NULL){
		print_string_at("Task Creation Failed!", 0, 0, 0x00FF0000);
		while(1){
			asm volatile("cli; hlt");
		}
	}
	uint8_t* stack_base = (uint8_t*)my_malloc(TASK_STACK_SPACE);
	uint32_t* stack_pointer = (uint32_t*)((uint8_t*)stack_base + TASK_STACK_SPACE);
	
	*(--stack_pointer) = EFLAGS;
	*(--stack_pointer) = GDT_CODE_SEGMENT;
	*(--stack_pointer) = (uint32_t)entry_point;

	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;
	*(--stack_pointer) = 0;

	new_task->base = (uint32_t*)stack_base;
	new_task->esp = (uint32_t)stack_pointer;
	new_task->pid = next_pid++;
	new_task->state = TASK_READY;
	kstrcpy(new_task->name, name);
	new_task->waiting_on_pid = -1;

	new_task->next = head_task;
	new_task->prev = tail_task;
	tail_task->next = new_task;
	head_task->prev = new_task;
	tail_task = new_task;

	return new_task;	
}

uint32_t Schedule(uint32_t current_esp)
{
	if (!current_task) return current_esp;
	current_task->esp = current_esp;

	if (current_task->state == TASK_RUNNING){
		current_task->state = TASK_READY;
	}

	TaskControlBlock_t* start_task = current_task;	
	TaskControlBlock_t* next_task = current_task->next;
	int found = 0;
	do {
		if (next_task->state == TASK_READY){
			found = 1;
			break;
		}
		next_task = next_task->next;
	}while (next_task != start_task);
	
	if (!found){
		next_task = current_task;
	}
		
	next_task->state = TASK_RUNNING;
	current_task = next_task;
	return next_task->esp;
}

void KillTask(TaskControlBlock_t* task)
{
	if (task == NULL){
		current_task->state = TASK_ZOMBIE;
		TaskControlBlock_t* parent_task = current_task->next;
		while (parent_task != current_task){
			if (parent_task->waiting_on_pid == current_task->pid){
				parent_task->state = TASK_READY;
				parent_task->waiting_on_pid = -1;
				break;
			}
			parent_task = parent_task->next;
		}
	}else if (task){
		TaskControlBlock_t* target_task = task;
		target_task->state = TASK_ZOMBIE;

		TaskControlBlock_t* parent_task = head_task;
		do {
			if (parent_task->waiting_on_pid == target_task->pid){
				parent_task->state = TASK_READY;
				parent_task->waiting_on_pid = -1;
				break;
			}
			parent_task = parent_task->next;
		}while(parent_task != head_task);
	}

	asm volatile("int $0x20");
}

void set_task_state(int pid, TaskState state)
{
	TaskControlBlock_t* temp = head_task;
	do {
		if (temp->pid == pid){
			temp->state = state;
			break;
		}
		temp = temp->next;
	}while(temp != head_task);
}


