ENTRY (init)

SECTIONS
{

    . = 0x00200000;

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
}