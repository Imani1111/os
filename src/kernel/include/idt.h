#ifndef IDT_H
#define IDT_H

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;


#define IDT_ENTRIES 256

#define GDT_CODE_SEGMENT 0x08
#define GDT_DATA_SEGMENT 0x10

#define IDT_FLAG_INTERRUPT_GATE 0x8E

#define PIC1_COMMAND 0x20
#define PIC1_DATA 0x21
#define PIC2_COMMAND 0xA0
#define PIC2_DATA 0xA1
#define PIC_EOI 0x20

typedef struct __attribute__((packed)){
	uint16_t israddr_low;
	uint16_t segment_selector;
	uint8_t reserved;
	uint8_t typeattr;
	uint16_t israddr_high;
}IDTEntry_t;

typedef struct __attribute__((packed)){
	uint16_t limit;
	uint32_t idtaddr;
}IDTPtr_t;

extern IDTEntry_t idt[IDT_ENTRIES];
extern IDTPtr_t idt_addr;

void send_byte_to_port(uint16_t port, uint8_t val);
uint8_t read_byte_from_port(uint16_t port);
uint16_t read_word_from_port(uint16_t port);
void send_word_to_port(uint16_t port, uint16_t val);
void CreateIDTEntry(IDTEntry_t* idt, uint32_t idtaddr, uint16_t segselector, uint8_t flags);
void init_idt(void);
void PIC_sendEOI(uint8_t irqno);

#endif
