use alloc::alloc::{GlobalAlloc, Layout};
use core::ffi::c_void;

extern "C" {
    fn kmalloc_common(size: usize, align: usize) -> *mut c_void;
    fn kfree(ptr: *mut c_void);
}

#[global_allocator]
static ALLOCATOR: Allocator = Allocator;

pub struct Allocator;
unsafe impl GlobalAlloc for Allocator {
    unsafe fn alloc(&self, layout: Layout) -> *mut u8 {
        let size = layout.size();
        let ptr = kmalloc_common(size, 0);
        
        if ptr.is_null() {
            panic!("Failed to allocate memory");
        }
        ptr as *mut u8
    }

    unsafe fn dealloc(&self, ptr: *mut u8, _layout: Layout) {
        kfree(ptr as *mut c_void);
    }
}