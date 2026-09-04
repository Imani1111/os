#include <mem_mgr.h>
#include <mmap.h>
#include <screen.h>

static HeapBlockMetaData* heap_start = NULL;
static HeapBlockMetaData* heap_tail = NULL;
extern uint32_t first_allocatable_addr;
HeapBlockMetaData* heap_pointer = NULL;
uint32_t heap_current = 0x400000;

// TODO: Fix malloc page fault when a big chunk is requested when i wake up
void* my_malloc(uint32_t size)
{
	if (size == 0) return NULL;
	uint32_t aligned = (size + 3) & ~3;
	
	if (heap_start == NULL){
		heap_start = (HeapBlockMetaData*)sbrk(PAGE);
		heap_start->size = PAGE - sizeof(HeapBlockMetaData);
		heap_start->is_free = 1;
		heap_start->next = NULL;
		heap_start->prev = NULL;

		heap_tail = heap_start;
		heap_pointer = heap_start;
	}

	HeapBlockMetaData* curr = heap_pointer;
	do {
		if (curr->is_free && curr->size >= aligned){
			HeapBlockMetaData* new = (HeapBlockMetaData*)((uint8_t*)curr + aligned + sizeof(HeapBlockMetaData));
			new->size = curr->size - aligned - sizeof(HeapBlockMetaData);
			new->is_free = 1;

			// TODO: Implement heap_tail logic
			if (curr->next == NULL){
				new->next = NULL;
				heap_tail = new;
			}else{
				new->next = curr->next;
				curr->next->prev = new;
			}
			new->prev = curr;
			
			curr->next = new;
			curr->size = aligned;
			curr->is_free = 0;

			heap_pointer = curr;

			void* ptr = (uint8_t*)curr + sizeof(HeapBlockMetaData);
			return ptr;
		}
		curr = curr->next;
	}while (curr != heap_pointer);

	//TODO: Implement heap expansion using sbrk when no block fits
	if ( heap_tail->is_free){
		uint32_t no_of_pages = ((aligned - heap_tail->size) + PAGE - 1) / PAGE;
		print_hex(no_of_pages, 0, 40);
		
		for (uint32_t i = 0; i < no_of_pages; i++){
			void* ptr = sbrk(PAGE);
			if (ptr == SBRK_FAIL) return NULL;
			heap_tail->size += PAGE;
		}
		
	}else{
		uint32_t no_of_pages = ((aligned + sizeof(HeapBlockMetaData)) + PAGE - 1) / PAGE;
		print_hex(no_of_pages, 0, 40);
		
		HeapBlockMetaData* expansion = NULL;
		
		for (uint32_t i = 0; i < no_of_pages; i++){
			if (expansion == NULL){
				expansion = (HeapBlockMetaData*)sbrk(PAGE);
				if (expansion == SBRK_FAIL) return NULL;
				
				expansion->next = NULL;
				expansion->prev = heap_tail;
				heap_tail->next = expansion;
				expansion->size = PAGE - sizeof(HeapBlockMetaData);
				expansion->is_free = 1;
			}
			else{
				void* ptr = sbrk(PAGE);
				if (ptr == SBRK_FAIL) return NULL;
				expansion->size += PAGE;
			}
		}
	}
	return my_malloc(aligned);
}

void my_free(void* ptr)
{
}

void* sbrk(int32_t increment)
{
	if (increment == 0) return (void*)heap_current;
	uint32_t old = heap_current;
	uint32_t new = heap_current + increment;
	if (increment > 0){
		uint32_t current_block_end = (old + PAGE - 1) & ~(PAGE - 1);
		uint32_t target_block_end = (new + PAGE - 1) & ~(PAGE - 1);
		
		while (current_block_end < target_block_end){
			uint32_t phys_addr = pmm_alloc_page();
			
			if (!phys_addr){
				return SBRK_FAIL;
			}

			uint32_t* addr = (uint32_t*)vmm_map_page(current_block_end, phys_addr, READ_WRITE);
			if (addr == MAP_FAILED) return SBRK_FAIL;
			
			current_block_end += PAGE;
		}
	}
	heap_current = new;
	return (void*)old;
}
