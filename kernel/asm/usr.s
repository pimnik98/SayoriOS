/*
 * SayoriOS
 * Ring 3 support
 */
.set        USER_CS,        0x1B    /* user mode code selector */   
.set        USER_SS,        0x23    /* user mode stack selector */
.set        USER_DS,        0x23    /* user mode data selector */

.global user_mode_switch

user_mode_switch:
    #cli
    
    mov     4(%esp), %edx
    
    mov     $USER_DS, %ax
    mov     %ax, %ds
    mov     %ax, %es
    mov     %ax, %fs
    mov     %ax, %gs

    #mov     8(%esp), %eax

    mov     8(%esp), %eax
    pushl   $USER_SS
    pushl   %eax
    pushf

    pushl $USER_CS
    pushl %edx

    #pop %eax
    #orl $0x200, %eax
    #push %eax

    #push    $USER_CS
    #push    %edx

    cli

    iret
