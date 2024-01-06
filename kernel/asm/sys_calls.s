/*
 * SayoriOS
 * Вызов системной функции
 */
.global     syscall_entry_call

syscall_entry_call:

        push    %edx
        push    %ecx
        push    %ebx
        
        mov    16(%esp), %edx
        
        call    *%edx           
        
        add     $12, %esp
        
        ret


