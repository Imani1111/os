#include <mmap.h>
#include <screen.h>
#include <string.h>

static PMM_stack_alloc pmm_stack_alloc;
extern uint8_t _kernel_end[];
uint32_t first_allocatable_addr = 0;

page_directory_t pd = {0};

page_directory_t boot_page_dir = {0};
page_table_t boot_page_t = {0};
page_table_t fb = {0};
page_table_t temp_heap = {0};
page_table_t pt = {0};

void init_pmm()
{
	MemoryMapEntry* mmap = (MemoryMapEntry*)MMAP_ENTRY_BUFFER;
	uint16_t* entry_count = (uint16_t*)ENTRY_COUNT_PTR;
	uint16_t total_entries = *entry_count;
	
	pmm_stack_alloc.capacity = MAX_PAGES;
	pmm_stack_alloc.stack_pointer = 0;
	
	first_allocatable_addr = (uint32_t)&pmm_stack_alloc.page_addresses + (MAX_PAGES * sizeof(uint32_t));
	for (uint16_t i = 0; i < total_entries; i++){
		if (mmap[i].type != 1){
			continue;
		}

		for (uint32_t page = mmap[i].base_addr; page < (mmap[i].base_addr + mmap[i].length); page += PAGE){
			page = (page + (PAGE- 1)) & ~(PAGE - 1);
			if (page >= 0x100000 && page >= first_allocatable_addr && page < 0xFFE0000){
				if (pmm_stack_alloc.stack_pointer < pmm_stack_alloc.capacity){
					pmm_stack_alloc.page_addresses[pmm_stack_alloc.stack_pointer] = page;
					pmm_stack_alloc.stack_pointer++;
				}
				else{
					continue;
				}
			}
		}
	}
}

uint32_t pmm_alloc_page()
{
	if (pmm_stack_alloc.stack_pointer == 0){
		return 0;
	}
	uint32_t page = pmm_stack_alloc.page_addresses[--pmm_stack_alloc.stack_pointer];
	return page; 
}

void pmm_free_page(uint32_t page_address)
{
	if (pmm_stack_alloc.stack_pointer < pmm_stack_alloc.capacity){
		pmm_stack_alloc.page_addresses[pmm_stack_alloc.stack_pointer++] = page_address;
	}
}

uint32_t pmm_get_free_page_count()
{
	return pmm_stack_alloc.stack_pointer;
}

void* vmm_map_page(uint32_t virtual_addr, uint32_t phys_addr, uint32_t flags)
{
	uint32_t pd_index = (virtual_addr >> 22) & 0x3FF;
	uint32_t pt_index = (virtual_addr >> 12) & 0x3FF;

	uint32_t pd_entry = pd.page_directory_entries[pd_index];
	uint32_t pt_phys;
	page_table_t* pt;

	if (!(pd_entry & PAGE_PRESENT)){
		pt_phys = pmm_alloc_page();
		pd.page_directory_entries[pd_index] = (uint32_t)pt_phys | PAGE_PRESENT | flags;

		pt = (page_table_t*)(MASTER_DIR_VIRTUAL_ADDR + (pt_index << 12));
		kmemset((void*)pt, 0, sizeof(page_table_t));
	}
	else{
		pt = (page_table_t*)(pd_entry & 0xFFFFF000);
	}
	pt->page_table_entries[pt_index] = phys_addr | PAGE_PRESENT | flags;
	asm volatile ("invlpg (%0)" : : "r" (virtual_addr) : "memory");
	return (void*)virtual_addr;
}

void enable_paging(uint32_t page_directory_address)
{
	asm volatile(
		"mov %0, %%cr3\n\t"
		"mov %%cr0, %%eax\n\t"
		"or $0x80000000, %%eax\n\t"
		"mov %%eax, %%cr0\n\t"
		: : "r" (page_directory_address) : "eax", "memory"
	);
}

void init_identity_mapping()
{
	for (int i = 0; i < NO_OF_ENTRIES; i++){
		uint32_t physical_addr = 0x1000 * i;
		boot_page_t.page_table_entries[i] = physical_addr | PAGE_PRESENT | USER_SUPERVISOR;
	}
	boot_page_dir.page_directory_entries[0] = ((uint32_t)boot_page_t.page_table_entries) | PAGE_PRESENT | READ_WRITE | 
		USER_SUPERVISOR;

	uint32_t base_addr = 0xfd000000;
	uint32_t size = 640 * 480 * 3;
	uint32_t pd_index  = base_addr >> 22;
	
	uint32_t no_of_pages = (size + PAGE - 1) / PAGE;	
	for (uint32_t j = 0; j < no_of_pages; j++){
		uint32_t page_phys = base_addr + (j * PAGE);
		fb.page_table_entries[j] = page_phys | PAGE_PRESENT | READ_WRITE;
	}
	boot_page_dir.page_directory_entries[pd_index] = ((uint32_t)fb.page_table_entries) | PAGE_PRESENT | READ_WRITE;

	uint32_t heap_start = 0x400000;
	uint32_t ssize = 0xFFE0000 - 0x400000;
	uint32_t ppd_index = heap_start >> 22;
	uint32_t no_of_ppages = (ssize + PAGE - 1) / PAGE;
	for (uint32_t k = 0; k < NO_OF_ENTRIES; k++){
		uint32_t phys_addr = pmm_alloc_page();
		temp_heap.page_table_entries[k] = phys_addr | PAGE_PRESENT | READ_WRITE;
	}
	boot_page_dir.page_directory_entries[ppd_index] = ((uint32_t)temp_heap.page_table_entries) | PAGE_PRESENT | READ_WRITE;
}

void init_recursive_mapping(){
	pd.page_directory_entries[1023] = (uint32_t)&pd;
}

void map_kernel(){
	pd.page_directory_entries[0] = boot_page_dir.page_directory_entries[0];
}

void map_fb(){
	uint32_t index = (0xfd000000 >> 22);
	pd.page_directory_entries[index] = boot_page_dir.page_directory_entries[index];
}

void load_master_dir()
{
	uint32_t master_dir = (uint32_t)&pd;
	asm volatile("mov %0, %%cr3" :: "r" (master_dir) : "memory");
}
void init_vmm()
{
	init_identity_mapping();
	enable_paging((uint32_t)&boot_page_dir);
	map_kernel();
	map_fb();
	load_master_dir();
	init_recursive_mapping();
}

