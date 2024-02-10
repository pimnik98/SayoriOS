//
// Created by ndraey on 2/10/24.
//

#pragma once

#include "multiboot.h"

void grub_modules_prescan(multiboot_header_t* hdr);
void grub_modules_init(multiboot_header_t* hdr);