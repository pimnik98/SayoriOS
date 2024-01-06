use core::fmt;
use core::fmt::Write;
use alloc::string::String;
use lazy_static::lazy_static;
use spin::Mutex;

pub mod rgb_image;

pub static PORT_COM1: u16 = 0x3f8;
pub static PORT_COM2: u16 = 0x2F8;
pub static PORT_COM3: u16 = 0x3E8;
pub static PORT_COM4: u16 = 0x2E8;
pub static PORT_COM5: u16 = 0x5F8;
pub static PORT_COM6: u16 = 0x4F8;
pub static PORT_COM7: u16 = 0x5E8;
pub static PORT_COM8: u16 = 0x4E8;

extern "C" {
    fn __com_writeString(port: u16, buf: *const u8);
}

#[macro_export]
macro_rules! qemu_print {
    ($($arg:tt)*) => ($crate::std::io::_print_qemu(format_args!($($arg)*)));
}

#[macro_export]
macro_rules! qemu_println {
    () => ($crate::print!("\n"));
    ($($arg:tt)*) => ($crate::qemu_print!("{}\n", format_args!($($arg)*)));
}

#[macro_export]
macro_rules! qemu_log {
    ($prefix:expr, $color:expr, $($arg:tt)*) => {
        let file = file!();
        let function = {
            fn f() {}
            fn type_name_of<T>(_: T) -> &'static str {
                core::any::type_name::<T>()
            }
            type_name_of(f).strip_suffix("::f").unwrap()
        };
        let line = line!();

        $crate::qemu_print!("[\x1b[{};1m{}\x1b[{};0m] {} [rust/{}:{}]\x1b[{};1m {} \x1b[{};0m\n",
            $color, $prefix, $color,
            function,
            file,
            line,
            $color,
            format_args!($($arg)*),
            $color
        )
    };
    ($($arg:tt)*) => {
        $crate::qemu_log!("LOG", "0", $($arg)*)
    };
}

#[macro_export]
macro_rules! qemu_warn {
    ($($arg:tt)*) => {
        $crate::qemu_log!("WARN", "33", $($arg)*)
    };
}

#[macro_export]
macro_rules! qemu_err {
    ($($arg:tt)*) => {
        $crate::qemu_log!("ERROR", "31", $($arg)*)
    };
}

#[macro_export]
macro_rules! qemu_ok {
    ($($arg:tt)*) => {
        $crate::qemu_log!("OK", "32", $($arg)*)
    };
}

#[macro_export]
macro_rules! qemu_note {
    ($($arg:tt)*) => {
        $crate::qemu_log!("NOTE", "36", $($arg)*)
    };
}

#[doc(hidden)]
pub fn _print_qemu(args: fmt::Arguments) {
    QEMU.lock().write_fmt(args).unwrap();
}

lazy_static! {
    pub static ref QEMU: Mutex<SerialWriter> = Mutex::new(SerialWriter {
        port: PORT_COM1
    });
}

pub struct SerialWriter {
    pub port: u16
}

impl fmt::Write for SerialWriter {
    fn write_str(&mut self, s: &str) -> fmt::Result {
        let mut buffer = String::from(s);
        buffer.push('\x00');
        unsafe {
            __com_writeString(self.port, buffer.as_str().as_bytes().as_ptr() as *const u8);
        }
        Ok(())
    }
}
