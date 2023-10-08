#include "kernel.h"
#include "sys/unwind.h"
#include "io/ports.h"

__attribute__((section(".debug_symbols"))) char* function_addr_data[128 * 1024] = {0};

char _temp_funcname[1024] = {0};

size_t decode_hex(char s[], int length) {
    int dec = 0;
    int power = 1;

    for (int i = length - 1; i >= 0; i--) {
        int digit = 0;

        if(s[i] >= 0 && s[i] <= '9') {
            digit = s[i] - '0';
        } else if(s[i] >= 'a' && s[i] <= 'f') {
            digit = s[i] - 'a' + 10;
        }

        dec += digit * power;
        power *= 16;
    }
    
    return dec;
}

// Returns true if okay, function name stored in _temp_funcname
bool get_func_name_by_addr(size_t addr) {
    char* temp = (char*)function_addr_data;

    memset(_temp_funcname, 0, 1024);

    do {
        memset(_temp_funcname, 0, 1024);
    
        size_t current_addr = decode_hex(temp, 8); // First addr
        
        temp += 8; // Address in HEX
        temp += 3; // Type

        memcpy(_temp_funcname, temp, struntil(temp, '\n') - 1);

        temp += struntil(temp, '\n'); // Name

        size_t next_addr = decode_hex(temp, 8); // Second addr
        
        if(addr >= current_addr && addr < next_addr) {
            return true;
        } else {
            // qemu_log("%x | %x | %x", current_addr, addr, next_addr);
        }
    } while(*temp != '\0');

    return false;
}

void unwind_stack(uint32_t MaxFrames) {
    struct stackframe *stk;
    asm ("movl %%ebp, %0" : "=r"(stk) :: );

    qemu_log("Stack trace:");

    for(uint32_t frame = 0; stk && frame < MaxFrames; ++frame) {
        qemu_printf("  Frame #%d: %x  =>  ", frame, stk->eip);

        bool exists = get_func_name_by_addr(stk->eip);

        qemu_printf("%s\n", exists ? _temp_funcname : "???");

        stk = stk->ebp;
    }

    // qemu_log("%s", function_addr_data);
}
