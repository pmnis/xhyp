/*
 * XHYP loader script
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY
{
		ram :   org = 0x02100000 , l = 0x00f00000
}

__stack_end__ = 0x03000000 - 0x10;


SECTIONS
{
	. = 0x02100000;

	. = ALIGN(4);
	.text : {
		main.o (.text)
		* (.text)
		* (.data)
		* (.rodata)
		* (.bss)
		* (.glue_7)
		* (.glue_7t)
	} > ram  =0x90909090 

}
