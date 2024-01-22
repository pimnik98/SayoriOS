.global	gdt_flush
gdt_flush:
    /* Load GDT */
    mov    4(%esp), %eax
    lgdt   (%eax)
    /*
    mov %cr0, %eax
    or $1, %al
    mov %eax, %cr0
    */
    mov	   $0x10, %ax
    mov	   %ax, %ds
    mov	   %ax, %es
    mov	   %ax, %fs
    mov	   %ax, %gs
    mov	   %ax, %ss
    ljmp $0x08,$flush
flush:
    ret

.global tss_flush
tss_flush:
    mov    4(%esp), %eax
    ltr	   %ax
    ret

.global idt_flush
idt_flush:
    mov	    4(%esp), %eax
    lidt    (%eax)
    ret
