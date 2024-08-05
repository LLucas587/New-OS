# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot


# Declare a header as in the Multiboot Standard.
.section .multiboot
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# export PATH="$HOME/opt/cross/bin:$PATH"

# Reserve a stack for the initial thread.
.section .bss
.align 16
stack_bottom:
.skip 16384 # 16 KiB
stack_top:


# The kernel entry point.
.section .text
.global _start
.type _start, @function
_start:
	# Setup Paging
	
	movl $(initial_page_dir - 0xC0000000), %eax # Moved code to higher half, therefore subtract by this address
	movl %eax, %cr3

	#turning on physical address extension
	movl %cr4,%ecx
	or $0x10, %ecx
	movl %ecx, %cr4

	#enable paging
	movl %cr0, %ecx
	orl $0x10, %ecx
	movl %ecx, %cr0

	#set write protect bit
	movl %cr0, %eax
	orl $0x10000, %eax   # Set the WP bit in CR0
	movl %eax, %cr0

	movl $stack_top, %esp

	# Call the global constructors.
	call _init

	#Call gdt intializer
	call init_gdt

	#Call idt initializer
	call idt_init

	# Transfer control to the main kernel.
	call kernel_main


	# Hang if kernel_main unexpectedly returns.
	cli
1:	hlt
	jmp 1b
.size _start, . - _start

.section .data
.align 4096
.global initial_page_dir
initial_page_dir:
	.long 0x00000013
	.rept 768-1 
	.long 0
	.endr

	.long (0 << 22) | 0x00000013
    .long (1 << 22) | 0x00000013
    .long (2 << 22) | 0x00000013
    .long (3 << 22) | 0x00000013
    .rept 256-4
    .long 0
    .endr
