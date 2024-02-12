// note: возможно лучшим решением будет переделать под определенные файловые системмы, но пока сойдет и так

use alloc::string::String;
use core::ptr::null_mut;
use core::ffi::{c_void, CStr};

use alloc::vec::Vec;
use alloc::vec;

#[repr(C)]
struct File {
    path: *const i8,
    size: i32,
    fmode: u32,
    open: bool,
    pos: isize,
    err: u32,
}

extern "C" { 
    fn fopen(filename: *const u8, mode: *const u8) -> *mut File;
    fn fclose(stream: *mut File);
    fn fsize(stream: *mut File) -> usize;
    fn fread(stream: *mut File, count: isize, size: usize, buffer: *mut c_void) -> i32;
}

pub fn read_to_string(file_path: &str) -> Result<&str, &str> {
    let mut file_path_string = String::from(file_path);
    file_path_string.push('\0');

    let file = unsafe {
        fopen(
            file_path_string.as_bytes().as_ptr(),
            b"r\0".as_ptr()
        )
    };

    if file == null_mut() {
        return Err("Failed to open file.");
    }

    let size = unsafe { fsize(file) };
    let mut buffer: Vec<u8> = vec![0; size]; // Создаем буфер для строки
    let ptr = buffer.as_mut_ptr() as *mut c_void;

    unsafe {
        fread(file, 1, size, ptr);

        fclose(file);
    }

    let result = unsafe {
        CStr::from_ptr(buffer.as_ptr() as *const i8).to_str().unwrap()
    };

    Ok(result)
}