.code32

.global v8086_enable
v8086_enable:
    xor %eax, %eax

    // Get our flags
    pushfl

    // Into eax
    pop %eax

    // Set VM bit
    orl $(1 << 17), %eax
    
    // Set ring 3
    orl $(3 << 12), %eax

    // Push our modified flags
    push %eax

    // And set our flags
    popfl

    // But pushf* and popf* instructions are user-space
    // We need to use iret to perform changes

    // Why this fuck causes CPU Reset?
    iret
