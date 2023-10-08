char fxsave_region[512] __attribute__((aligned(16))) = {0};

void __wtf_fxsave() {
    __asm__ volatile("fxsave %0 " :: "m"(fxsave_region));
}