pub mod allocator;

pub const PAGE_SIZE: u32 = 0x1000;
pub const PAGE_PRESENT: u32 =	1 << 0;
pub const PAGE_WRITEABLE: u32 = 1 << 1;
pub const PAGE_USER: u32 = 1 << 2;
pub const PAGE_WRITE_THROUGH: u32 = 1 << 3;
pub const PAGE_CACHE_DISABLE: u32 = 1 << 4;
pub const PAGE_ACCESSED: u32 = 1 << 5;
pub const PAGE_DIRTY: u32 = 1 << 6;
pub const PAGE_GLOBAL: u32 = 1 << 8;


extern "C" {
    // void map_pages(uint32_t* page_dir, physical_addr_t physical, virtual_addr_t virtual, size_t size, uint32_t flags);

    pub fn map_pages(page_dir: *mut u32, physical_mem: u32, virtual_mem: u32, size_bytes: u32, flags: u32);

    // uint32_t* get_kernel_page_directory();

    pub fn get_kernel_page_directory() -> *mut u32;
}