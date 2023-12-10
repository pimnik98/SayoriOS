/*------------------------------------------------------------------------------
//
//      Multiboot header
//
//----------------------------------------------------------------------------*/
#ifndef		MULTIBOOT_H
#define		MULTIBOOT_H

#include	"common.h"

#define		MBOOT_FLAG_MEM		0x001
#define		MBOOT_FLAG_DEVICE	0x002
#define		MBOOT_FLAG_CMDLINE	0x004
#define		MBOOT_FLAG_MODS		0x008
#define		MBOOT_FLAG_AOUT		0x010
#define		MBOOT_FLAG_ELF		0x020
#define		MBOOT_FLAG_MMAP		0x040
#define		MBOOT_FLAG_CONFIG	0x080
#define		MBOOT_FLAG_LOADER	0x100
#define		MBOOT_FLAG_APM		0x200
#define		MBOOT_FLAG_VBE		0x400

struct	multiboot_header
{
  uint32_t	flags;          /* Header flags */
  uint32_t	mem_lower;
  uint32_t	mem_upper;
  uint32_t	boot_device;
  uint32_t	cmdline;
  uint32_t	mods_count;
  uint32_t	mods_addr;
  uint32_t	num;
  uint32_t	size;
  uint32_t	addr;
  uint32_t	shndx;
  uint32_t	mmap_length;
  uint32_t	mmap_addr;
  uint32_t	drives_length;
  uint32_t	drives_addr;
  uint32_t	config_table;
  uint32_t	boot_loader_name;
  uint32_t	apm_table;
  uint32_t	vbe_control_info;
  uint32_t	vbe_mode_info;
  uint16_t	vbe_mode;
  uint16_t	vbe_interface_seg;
  uint16_t	vbe_interface_off;
  uint16_t	vbe_interface_len;

  uint64_t framebuffer_addr;
  uint32_t framebuffer_pitch;
  uint32_t framebuffer_width;
  uint32_t framebuffer_height;
  uint8_t framebuffer_bpp;
  uint8_t framebuffer_type;

  union {
      struct {
          uint32_t framebuffer_palette_addr;
          uint16_t framebuffer_palette_num_colors;
      };

      struct {
          uint8_t framebuffer_red_field_position;
          uint8_t framebuffer_red_mask_size;
          uint8_t framebuffer_green_field_position;
          uint8_t framebuffer_green_mask_size;
          uint8_t framebuffer_blue_field_position;
          uint8_t framebuffer_blue_mask_size;
      };
  };
  
}__attribute__((packed));

typedef	struct	multiboot_header multiboot_header_t;

struct multiboot_mod_list
{
  /* the memory used goes from bytes ’mod_start’ to ’mod_end-1’ inclusive */
  uint32_t mod_start;
  uint32_t mod_end;

  /* Module command line */
  uint32_t cmdline;
};
typedef struct multiboot_mod_list multiboot_module_t;


/* APM BIOS info. */
struct multiboot_apm_info
{
  uint16_t version;
  uint16_t cseg;
  uint32_t offset;
  uint16_t cseg_16;
  uint16_t dseg;
  uint16_t flags;
  uint16_t cseg_len;
  uint16_t cseg_16_len;
  uint16_t dseg_len;
};

struct memory_map_entry
{
	uint32_t		size;
	uint32_t		addr_low;
	uint32_t		addr_high;
	uint32_t		len_low;
	uint32_t		len_high;
	uint32_t		type;

}__attribute__((packed));

typedef struct memory_map_entry memory_map_entry_t;

struct multiboot_tag_module
{
  uint32_t type;
  uint32_t size;
  uint32_t mod_start;
  uint32_t mod_end;
  char cmdline[0];
}__attribute__((packed));

typedef struct multiboot_tag_module multiboot_tag_module_t;

#endif
