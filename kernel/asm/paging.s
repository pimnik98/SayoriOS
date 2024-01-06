.text
.globl load_page_directory
load_page_directory:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    mov %eax, %cr3
    mov %ebp, %esp
    pop %ebp
    ret

.text
.globl enable_paging
enable_paging:
    push %ebp
    mov %esp, %ebp
    mov %cr0, %eax
    or $0x80000000, %eax
    mov %eax, %cr0
    mov %ebp, %esp
    pop %ebp
    ret

.text
.globl flush_tlb_entry
flush_tlb_entry:
    push %ebp
    mov %esp, %ebp
    mov 8(%esp), %eax
    invlpg   (%eax)
    mov %ebp, %esp
    pop %ebp
    ret
