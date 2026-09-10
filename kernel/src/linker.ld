ENTRY(_start)

PHDRS
{
	text PT_LOAD ;
	rodata PT_LOAD ;
	data PT_LOAD ;
	userdata PT_LOAD ;
	symbols PT_LOAD ;
}

SECTIONS
{
	__kernel_mem = 0xffff800000000000;

	. = 0xffffffffc0000000;

	.text : ALIGN(4K)
	{
		*(.text*)
	} :text

	.rodata : ALIGN(4K)
	{
		*(.rodata*)
	} :rodata

	.data : ALIGN(4K)
	{
		*(.data*)
	} :data

	.bss : ALIGN(4K)
	{
		*(COMMON)
		*(.bss*)
	} :data

	. = ALIGN(4K);

	.usertext : ALIGN(4K)
	{
		__usertext = .;
		*(.usertext)
		*(.userconst)
		__usertext_end = .;
	} :userdata

	.symbols : ALIGN(4K)
	{
		*(.symbols)
	} :symbols
}
