#![no_std]
#![no_main]


extern crate alloc;

use core::panic::PanicInfo;

pub mod ffi;

use ffi::tty::*;

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    println!("{}", _info);
    tty_puts("panek");
    loop {}
}

#[no_mangle]
#[allow(improper_ctypes_definitions)]
pub extern "C" fn rust_command(_argc: u32, _argv: &[*const u8]) {
    tty_puts("First program written on rust.");
}

#[no_mangle]
pub extern "C" fn rust_main() {
    println!("Hello, {}!", "Rust");
    panic!("test rust panic!");
    tty_puts("Hey guys did you know...");
}
