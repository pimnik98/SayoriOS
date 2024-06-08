asm_sources = files('64bit_on_32bit.s', 'gdt.s', 'init.s', 'interrupt.s', 'paging.s', 'regs.s', 'sse.s', 'switch_task.s', 'sys_calls.s', 'usr.s')

# Compile assembly files to object files
asm_objs = []
foreach asm_file : asm_sources
  asm_objs += custom_target(asm_file.stem(),
    input : asm_file,
    output : asm_file.stem() + '.o',
    command : [asm, as_flags, '@INPUT@', '--32', '-o', '@OUTPUT@'],
    build_by_default : true)
endforeach

# Add C sources and assembly object files to the list
subdir_done(asm_sources)
