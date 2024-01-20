.section	.text
.global	sse_check

sse_check:
    mov $0x1, %eax

    cpuid

    test $(1 << 25), %edx
    mov $0x1, %eax

    jnz good

    xor %eax, %eax
good:
    ret


.global		sse_enable

sse_enable:
    # enable SSE
    mov %cr0, %eax
    and $~0x04, %ax
    or $0x2, %ax
    mov %eax, %cr0

    mov %cr4, %eax
    or $(3 << 9), %ax
    mov %eax, %cr4

    ret
