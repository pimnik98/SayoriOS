ENTRY (__pre_init)

SECTIONS
{
    . = 0x00200000;
    KERNEL_BASE_pos = .;

    .text ALIGN (0x1000) :
    {
        *(.mboot)
        *(.text)
    }

    .data ALIGN (0x1000) :
    {
        *(.data)
    }

    .bss :
    {
        *(.bss)
    }
    /* */
    end = .; _end = .; __end = .; 
    KERNEL_END_pos = .;
}
