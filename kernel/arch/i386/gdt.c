#include <stdint.h>

// Define GDT entry structure
struct gdt_entry {
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t base_middle;
    uint8_t access;
    uint8_t granularity;
    uint8_t base_high;
} __attribute__((packed));

// Define GDT pointer structure
struct gdt_ptr {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed));

// TSS structure
struct tss_entry {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed));

// Define GDT entries and pointer
struct gdt_entry gdt[6];
struct gdt_ptr gp;

// TSS entry
struct tss_entry tss;

// Function to set up a GDT entry
void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran) {
    gdt[num].base_low = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high = (base >> 24) & 0xFF;

    gdt[num].limit_low = (limit & 0xFFFF);
    gdt[num].granularity = ((limit >> 16) & 0x0F) | (gran & 0xF0);
    gdt[num].access = access;
}

// Function to write to the TSS
void write_tss(int num, uint32_t ss0, uint32_t esp0) {
    // Calculate the base and limit of the TSS entry
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss) - 1;

    // Add the TSS descriptor to the GDT
    set_gdt_entry(num, base, limit, 0x89, 0x00);

    // Initialize the TSS structure
    tss.ss0 = ss0;
    tss.esp0 = esp0;
    // Set the IO map base address to the end of the TSS
    tss.iomap_base = sizeof(tss);
}

// Function to initialize the GDT
void init_gdt() {
    gp.limit = (sizeof(gdt) - 1);
    gp.base = (uint32_t)&gdt;

    // Null segment
    set_gdt_entry(0, 0, 0, 0, 0);

    // Kernel mode code segment
    set_gdt_entry(1, 0, 0xFFFFF, 0x9A, 0xC0);

    // Kernel mode data segment
    set_gdt_entry(2, 0, 0xFFFFF, 0x92, 0xC0);

    // User mode code segment
    set_gdt_entry(3, 0, 0xFFFFF, 0xFA, 0xC0);

    // User mode data segment
    set_gdt_entry(4, 0, 0xFFFFF, 0xF2, 0xC0);

    // Task State Segment (TSS)
    write_tss(5, 0x10, 0);

    // Load the GDT using inline assembly
    asm volatile (
        "lgdt (%0)\n"          // Load the GDT
        "mov $0x10, %%ax\n"    // Load the data segment selector (index 2)
        "mov %%ax, %%ds\n"
        "mov %%ax, %%es\n"
        "mov %%ax, %%fs\n"
        "mov %%ax, %%gs\n"
        "mov %%ax, %%ss\n"
        "ljmp $0x08, $1f\n"    // Far jump to load the code segment selector (index 1)
        "1:\n"
        "mov $0x2B, %%ax\n"    // Load the TSS selector (index 5)
        "ltr %%ax\n"
        :
        : "r" (&gp)
        : "memory", "cc", "ax"
    ); 
}
