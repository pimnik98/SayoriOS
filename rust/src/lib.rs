#![no_std]
#![no_main]

extern crate alloc;

use core::panic::PanicInfo;

pub mod std;
pub mod other;
mod system;
mod drv;


use core::ffi::CStr;

// use alloc::vec::Vec;

// use crate::other::tinyada::{Ada, Token, TokenType};

#[panic_handler]
fn panic(_info: &PanicInfo) -> ! {
    println!("{}", _info);
    qemu_println!("{}", _info);
    loop {}
}

#[no_mangle]
pub extern "C" fn rust_command(_argc: u32, _argv: *const *const i8) -> u32 {
    unsafe {
        qemu_note!("argc is {}", _argc);
        for x in 0.._argc {
            let arg = _argv.wrapping_add(x as usize).read();
            qemu_note!("argv[{}] = {}", x, CStr::from_ptr(arg).to_str().unwrap());
        }
    }

	println!("Hello, World!");

//     let mut ada = Ada::new("with Ada.Text_IO\nprocedure Hello is\nbegin\nend Hello;".as_bytes());
// 
// 	qemu_note!("Ada created");
// 
//     ada.lexer();

    // qemu_note!("Ada tokens: {:#?}", ada.tokens);

    0
}

#[no_mangle]
pub extern "C" fn rust_main() {
    println!("Привет, {}!", "Rust");
}
