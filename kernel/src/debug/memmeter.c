#include "common.h"
#include "mem/vmm.h"
#include "io/ports.h"

void memmeter(void (*function)()) {
    size_t mem_before = system_heap.used_memory;
    size_t mem_before_cnt = system_heap.allocated_count;

    function();

    size_t mem_after = system_heap.used_memory;
    size_t mem_after_cnt = system_heap.allocated_count;

    int delta = (int)mem_after - (int)mem_before;

    qemu_warn("Memory delta: %d bytes (%d objects)",
              delta, mem_after_cnt - mem_before_cnt);

    if(delta > 0) {
        qemu_err("Memory leak!");
    } else if(delta == 0) {
        qemu_ok("No memory leak! :)");
    }
}
