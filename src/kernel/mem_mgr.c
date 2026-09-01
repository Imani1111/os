#include <mem_mgr.h>
#include <mmap.h>
#include <screen.h>

static HeapBlockMetaData* heap_start = NULL;
static HeapBlockMetaData* heap_tail = NULL;
extern uint32_t first_allocatable_addr;
HeapBlockMetaData* heap_pointer = NULL;
uint32_t heap_current = 0x400000;

void* my_malloc(uint32_t size_in_bytes)
{
	print_hex(heap_current, 0, 48);
	if (size_in_bytes == 0){
		return NULL;
	}
	if (heap_start == NULL){
		heap_start = (HeapBlockMetaData*)sbrk(PAGE);
		if (heap_start == (void*)-1) return NULL;
		heap_start->size = PAGE - (sizeof(HeapBlockMetaData));
		heap_start->is_free = 1;
		heap_start->next = NULL;
		heap_start->prev = NULL;
		heap_pointer = heap_start;
		heap_tail = heap_start;
		heap_current = (uint32_t)heap_start + PAGE;
	}
	HeapBlockMetaData* current = heap_start;
	while (current != NULL){
		if (current->is_free && current->size >= size_in_bytes){
			if (current->size >= size_in_bytes + sizeof(HeapBlockMetaData)){
				HeapBlockMetaData* new_block = (HeapBlockMetaData*)((uint8_t*)current + sizeof(HeapBlockMetaData) + current->size);
				new_block->prev = current;
				new_block->size = current->size - size_in_bytes - sizeof(HeapBlockMetaData);
				new_block->is_free = 1;
				new_block->next = current->next;
				if (current->next != NULL){
					current->next->prev = new_block;
				}else{
					heap_tail = new_block;
				}
				current->size = size_in_bytes;
				current->next = new_block;
			}
			current->is_free = 0;
			heap_pointer = current;
			
			void* payload = (void*)((uint8_t*)current + sizeof(HeapBlockMetaData));
			return payload;
		}
		current = current->next;
	}
	// TODO: Handle expansion via sbrk here when no block is found
	uint32_t tail_end_addr = (uint32_t)heap_tail + sizeof(HeapBlockMetaData) + heap_tail->size;
	if (tail_end_addr >= heap_current){
		if (heap_tail->is_free){
			void* res = sbrk(PAGE);
			if (res == (void*)-1) return NULL;
			heap_tail->size += PAGE;
		}else{
			HeapBlockMetaData* new_tail = (HeapBlockMetaData*)sbrk(PAGE);
			if (new_tail == (void*)-1) return NULL;
			new_tail->is_free = 1;
			new_tail->next = NULL;
			new_tail->prev = heap_tail;
			new_tail->prev->next = new_tail;
			new_tail->size = PAGE - sizeof(HeapBlockMetaData);
			heap_tail = new_tail;
		}
	}
	return my_malloc(size_in_bytes);
}

void my_free(void* ptr)
{
	if (ptr == NULL) return;

	HeapBlockMetaData* chunk_meta_data = (HeapBlockMetaData*)ptr - 1;
	chunk_meta_data->is_free = 1;

	if (chunk_meta_data->next != NULL && chunk_meta_data->next->is_free){
		chunk_meta_data->size += sizeof(HeapBlockMetaData) + chunk_meta_data->next->size;
		chunk_meta_data->next = chunk_meta_data->next->next;

		if (chunk_meta_data->next != NULL){
			chunk_meta_data->next->prev = chunk_meta_data;
		}
	}
	if (chunk_meta_data->prev != NULL && chunk_meta_data->prev->is_free){
		HeapBlockMetaData* left_block = chunk_meta_data->prev;
		left_block->size += sizeof(HeapBlockMetaData) + chunk_meta_data->size;

		left_block->next = chunk_meta_data->next;
		if (left_block->next != NULL){
			left_block->next->prev = left_block;
		}
	}	
}

void* sbrk(int32_t increment)
{
	if (increment == 0) return (void*)heap_current;
	uint32_t old = heap_current;
	uint32_t new = heap_current + increment;
	if (increment > 0){
		uint32_t current_page_end = (old + PAGE - 1) & ~(PAGE - 1);
		uint32_t target_page_end = (new + PAGE - 1) & ~(PAGE - 1);
		while (current_page_end < target_page_end){
			uint32_t phys_addr = pmm_alloc_page();
			if (!phys_addr){
				return (void*)-1;
			}
			uint32_t* addr = (uint32_t*)vmm_map_page(current_page_end, phys_addr, READ_WRITE);
			if (addr == MAP_FAILED) return SBRK_FAIL;
			current_page_end += PAGE;
		}
	}
	heap_current = new;
	return (void*)old;
}
