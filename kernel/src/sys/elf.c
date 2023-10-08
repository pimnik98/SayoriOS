/**
 * @file sys/elf.c
 * @author Пиминов Никита (nikita.piminoff@yandex.ru), NDRAEY >_ (pikachu_andrey@vk.com)
 * @brief Загрузщик ELF
 * @version 0.3.3
 * @date 2022-10-20
 * @copyright Copyright SayoriOS Team (c) 2022-2023
*/

#include <kernel.h>
#include <io/ports.h>
#include <lib/stdio.h>
#include <lib/math.h>
#include <elf/elf.h>
#include <sys/trigger.h>

uint32_t vmm_allocated[4096];
uint32_t vmm_mapped[4096];
uint32_t vmm_sizes[4096];

elf_sections_t* load_elf(const char* name){
	/* Open ELF file */
	FILE *file_elf = fopen(name, "r");

	if (file_elf->err) {
		qemu_log("Failed to open ELF file: %s / %d", name, file_elf->err);
		return nullptr;
	}

	/* Allocate ELF file structure */
	elf_sections_t* elf = (elf_sections_t*) kmalloc(sizeof(elf_sections_t));
	memset(elf, 0, sizeof(elf_sections_t));

	/* Read ELF header */
	elf->elf_header = (Elf32_Ehdr*) kmalloc(sizeof(Elf32_Ehdr));
	memset(elf->elf_header, 0, sizeof(Elf32_Ehdr));

	fread(file_elf, 1, sizeof(Elf32_Ehdr), elf->elf_header);

	/* Read program header */
	Elf32_Half proc_entries = elf->elf_header->e_phnum;
	uint32_t proc_size = elf->elf_header->e_phentsize;

	elf->p_header = (Elf32_Phdr*)kmalloc(proc_size*proc_entries);
	memset(elf->p_header, 0, proc_size*proc_entries);

	fseek(file_elf, (ssize_t)elf->elf_header->e_phoff, SEEK_SET);

	fread(file_elf, proc_entries, proc_size, elf->p_header);

	/* Read ELF sections */
	Elf32_Half sec_entries = elf->elf_header->e_shnum;

	elf->section = (Elf32_Shdr*) kmalloc(sizeof(Elf32_Shdr)*sec_entries);
	memset(elf->section, 0, sizeof(Elf32_Shdr)*sec_entries);

	fseek(file_elf, (ssize_t)elf->elf_header->e_shoff, SEEK_SET);

	fread(file_elf, sec_entries, sizeof(Elf32_Shdr), elf->section);

	elf->file = file_elf;

	return elf;
}

int32_t run_elf_file(const char *name, int32_t argc, char* eargv[]) {
    if (!vfs_exists(name)) {
        qemu_log("run_elf_file: elf [%s] does not exist", name);
		CallTrigger(0x0006,(void*)name,(void*)1,0,0,0);
        return -1;
    }

    elf_sections_t* elf_file = load_elf(name);
    if (elf_file == nullptr) {
		qemu_log("[DBG] Error opening file %s\n", name);
		CallTrigger(0x0006,(void*)name,(void*)2,0,0,0);
        return -1;
    }
	CallTrigger(0x0004,(void*)name,(void*)0,0,0,0);
	// char* data = kmalloc(elf_file->file->size);
	// fseek(elf_file->file, 0, SEEK_SET);

	// fread(elf_file->file, elf_file->file->size, 1, data);

	 qemu_log("Ident: %s", elf_file->elf_header->e_ident);
	 qemu_log("Type: %x", elf_file->elf_header->e_type);
	 qemu_log("Machine: %x", elf_file->elf_header->e_mashine);
	 qemu_log("Version: %x", elf_file->elf_header->e_version);
	 qemu_log("Entry point: %x", elf_file->elf_header->e_entry);
	 qemu_log("Program Header Offset: %x", elf_file->elf_header->e_phoff);
	 qemu_log("Section Header Offset: %x", elf_file->elf_header->e_shoff);
	 qemu_log("Flags: %x", elf_file->elf_header->e_flags);
	 qemu_log("Program Header Size: %d", elf_file->elf_header->e_phentsize);
	 qemu_log("Program Header Entries: %d", elf_file->elf_header->e_phnum);
	 qemu_log("Section Header Size: %d", elf_file->elf_header->e_shentsize);
	 qemu_log("Section Header Entries: %d", elf_file->elf_header->e_shnum);
	
	uint32_t vmm_allocated_count = 0;

    for (int32_t i = 0; i < elf_file->elf_header->e_phnum; i++) {
        Elf32_Phdr *phdr = elf_file->p_header + i;
		
		if (phdr->p_type != PT_LOAD)
			continue;

		// tty_printf(" - TYPE: %s  OFFSET: %x  VADDR: %x  PADDR:%x  FSIZE: %x  MSIZE: %x\n",
		// 			(phdr->p_type==PT_LOAD?"LOAD":"UNKNOWN"),
		// 			phdr->p_offset,
		// 			phdr->p_vaddr,
		// 			phdr->p_paddr,
		// 			phdr->p_filesz,
		// 			phdr->p_memsz);

        qemu_log("Loading %x bytes to %x", phdr->p_memsz, phdr->p_vaddr);

		size_t pagecount = MAX((phdr->p_memsz / PAGE_SIZE), 1);
		
		physaddr_t addrto = alloc_phys_pages(pagecount);
		qemu_log("Page count allocated now: %d (memsz: %d; PAGE SIZE: %d)",
				 pagecount, phdr->p_memsz, PAGE_SIZE);

		map_pages(
			get_kernel_dir(),
			phdr->p_vaddr,
			addrto,
			pagecount,
			(PAGE_PRESENT | PAGE_USER | PAGE_WRITEABLE) // 0x07
		);

		// Can be collapsed into: vmm_allocated[vmm_allocated_count++] = addrto;
		
		vmm_allocated[vmm_allocated_count] = addrto;
		vmm_sizes[vmm_allocated_count] = pagecount;
		vmm_mapped[vmm_allocated_count] = phdr->p_vaddr;
		vmm_allocated_count++;
		
        memset((void*)phdr->p_vaddr, 0, phdr->p_memsz);
		qemu_log("Set %x - %x to zero.", (int)((void*)phdr->p_vaddr), (int)((void*)phdr->p_vaddr) + phdr->p_memsz);

		fseek(elf_file->file, (ssize_t)phdr->p_offset, SEEK_SET);
		fread(elf_file->file, phdr->p_filesz, 1, (char *) phdr->p_vaddr);

        qemu_log("Loaded");
    }

    int(*entry_point)(int argc, char* eargv[]) = (int(*)(int, char**))elf_file->elf_header->e_entry;
    qemu_log("ELF entry point: %x", elf_file->elf_header->e_entry);

    qemu_log("Executing");
    int _result = entry_point(argc, eargv);
	
    qemu_log("[PROGRAMM FINISHED WITH CODE <%d>]", _result);
    qemu_log("Cleaning VMM:");

    for (int32_t i = 0; i < vmm_allocated_count; i++){
        qemu_log("\tCleaning %d: %x", i, vmm_allocated[i]);
		unmap_pages(get_kernel_dir(), vmm_mapped[i], vmm_sizes[i]);
        vmm_free_page(vmm_allocated[i]);
    }

    qemu_log("[CLEANED <%d> PAGES]", vmm_allocated_count);


	// FREE ELF DATA

	fclose(elf_file->file);

	kfree(elf_file->elf_header);
	kfree(elf_file->section);
	kfree(elf_file->p_header);
	
	CallTrigger(0x0005,(void*)name,(void*)_result,0,0,0);
    return 0;
}
