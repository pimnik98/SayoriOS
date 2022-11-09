/*
 * SayoriOS
 * Кольцо третий защиты
 */
.set        USER_CS,        0x1B    /* user mode code selector */   
.set        USER_SS,        0x23    /* user mode stack selector */
.set        USER_DS,        0x23    /* user mode data selector */

.global user_mode_switch

user_mode_switch:
  
    mov     4(%esp), %edx           
    
    mov     $USER_DS, %ax
    mov     %ax, %ds
    mov     %ax, %es
    
    mov     8(%esp), %eax   
    pushl   $USER_SS        
    pushl   %eax            
    pushf                   
    push    $USER_CS        
    push    %edx            
                            
    iret                    


