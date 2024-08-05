/* The bootloader will look at this image and start execution at the symbol
   designated at the entry point. */
ENTRY(_start)

MEMORY
{
  ROM (rx)  : ORIGIN = 0x0000000000100000, LENGTH = 0x0000000000001000  /* 4 KiB for ROM */
  RAM (rwx) : ORIGIN = 0x0000000000101000, LENGTH = 0x0000000000200000  /* 2 MiB for RAM */
}

/* Tell where the various sections of the object files will be put in the final
   kernel image. */
SECTIONS
{
	/* Begin putting sections at 1 MiB, a conventional place for kernels to be
	   loaded at by the bootloader. */
	. = 1M;

	/* First put the multiboot header, as it is required to be put very early
	   early in the image or the bootloader won't recognize the file format.
	   Next we'll put the .text section. */
	.text ALIGN(4K) :
	{
		__text_start = .;
		*(.multiboot)
		*(.text)
		__text_end = .;
	}>ROM /* Executable and Readable */

	/* Read-only data. */
	.rodata ALIGN(4K) :
	{
		__rodata_start = .;
		*(.rodata)
		__rodata_end = .;
	} >ROM /* Readable */

	/* Read-write data (initialized) */
	.data ALIGN(4K) :
	{
		__data_start = .;
		*(.data)
		__data_end = .;
	} >RAM /* Writable and Readable */

	/* Read-write data (uninitialized) and stack */
	.bss ALIGN(4K) :
	{
		__bss_start = .;
		*(COMMON)
		*(.bss)
		 __bss_end = .; 
	} >RAM /* Writable and Readable */

	__end = .;

}
