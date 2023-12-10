use core::fmt;
use core::fmt::Write;
use lazy_static::lazy_static;
use spin::Mutex;

use alloc::string::String;

extern "C" {
    fn _tty_puts(c: *const u8);
}

#[macro_export]
macro_rules! print {
    ($($arg:tt)*) => ($crate::ffi::tty::_print_tty(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! println {
    () => ($crate::print!("\n"));
    ($($arg:tt)*) => ($crate::print!("{}\n", format_args!($($arg)*)));
}

#[doc(hidden)]
pub fn _print_tty(args: fmt::Arguments) {
    WRITER.lock().write_fmt(args).unwrap();
}

pub fn tty_puts(s: &str) {
    let mut buffer = String::from(s);
    buffer.push('\x00');

    unsafe {
        _tty_puts(buffer.as_str().as_bytes().as_ptr() as *const u8);
    }
}

lazy_static! {
    pub static ref WRITER: Mutex<Writer> = Mutex::new(Writer);
}

pub struct Writer;
impl fmt::Write for Writer {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        tty_puts(s);
        Ok(())
    }
}
