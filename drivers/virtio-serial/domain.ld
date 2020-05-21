/*
 * XHYP loader script
 */

OUTPUT_FORMAT("elf32-littlearm", "elf32-littlearm", "elf32-littlearm")
OUTPUT_ARCH(arm)
ENTRY(_start)
MEMORY
{
		ram :   org = 0x02100000 , l = 0x00e00000
}

__stack_end__ = 0x02800000 - 0x10;


SECTIONS
{
	. = 0x02100000;

	. = ALIGN(4);
	.text : {
		main.o (.text)
		* (.text)
		. += 64k;
		* (.data)
		. += 64k;
		* (.rodata)
		. += 64k;
		* (.bss)
		. += 64k;
		* (.glue_7)
		* (.glue_7t)
	} > ram  =0x90909090 

}
