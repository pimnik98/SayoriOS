ENTRY (__pre_init)

SECTIONS
{
    . = 0x00200000;

    KERNEL_BASE_pos = .;

    .text ALIGN (0x1000) :
    {
        *(.mboot)

        CODE_start = .;
        
        *(.text*)

		CODE_end = .;
    }

    .data ALIGN (0x1000) :
    {
    	DATA_start = .;

        *(.data*)

		DATA_end = .;
    }

    .rodata ALIGN (0x1000) :
    {
    	RODATA_start = .;
    	
    	*(.rodata*)

		RODATA_end = .;
    }

    .bss ALIGN (0x1000) :
    {
    	*(COMMON)

		BSS_start = .;
		
        *(.bss*)

		BSS_end = .;
    }

    .debug_symbols ALIGN(0x1000) : {
    	*(.debug_symbols*)
    }

    .user ALIGN(0x1000) : {
    	USER_start = .;
    	*(.user*)
    	USER_end = .;
    }

    KERNEL_END_pos = .;
}
