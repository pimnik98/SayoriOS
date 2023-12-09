#![no_std]
#![no_main]

// TODO: write ffi allocator

use core::panic::PanicInfo;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    loop {}
}

extern "C" {
  fn _tty_puts(c: *const u8);
}

fn tty_puts(s: &str) {
  unsafe {
    _tty_puts(s.as_bytes().as_ptr() as *const u8);
  }
}

#[no_mangle]
pub extern "C" fn rust_command(_argc: u32, _argv: &[*const u8]) {
  tty_puts("First program written on rust.\x00");
}

#[no_mangle]
pub extern "C" fn rust_main() {
  tty_puts("Hello, Rust!\n\x00");
}