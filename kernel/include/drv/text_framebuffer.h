#ifndef		TEXT_FRAMEBUFFER_H
#define		TEXT_FRAMEBUFFER_H

#define		VIDEO_MEMORY	0xB8000

/*	Main colors		*/

#define		BLACK		0x0
#define		BLUE		0x1
#define		GREEN		0x2
#define		CIAN		0x3
#define		RED		0x4
#define		MAGENA		0x5
#define		BROWN		0x6

#define		LIGHT_GRAY	0x7
#define		DARK_GRAY	0x8
#define		LIGHT_BLUE	0x9
#define		LIGHT_GREEN	0xA
#define		LIGHT_CIAN	0xB
#define		LIGHT_RED	0xC
#define		LIGHT_MAGENA	0xD
#define		LIGHT_BROWN	0xE
#define		WHITE		0xF

#define		SCREEN_WIDTH	80
#define		SCREEN_HEIGHT	25

#include	"common.h"

/*------------------------------------------------------------------------------
//
//----------------------------------------------------------------------------*/
typedef struct
{
	uint8_t		cur_x;
	uint8_t		cur_y;
	uint16_t*		vmemory;

}__attribute__((packed)) vscreen_t;

  /* Move hardware cursor to reference screen position */
  void	move_cursor(uint8_t x, uint8_t y);
  
  /*	Set colors	*/
  void	set_bkground_color(uint8_t color);
  void	set_text_color(uint8_t color);

  /* Clean screen */
  void	clear(void);
  
  /* Print text string */
  void	print_text(char* s);
  
  /* Print hexadecimal value on screen */
  void	print_hex_value(uint64_t value);
  
  /* Print decimal value on screen */
  void	print_dec_value(uint32_t value);
  
  /* Print byte */
  void print_byte(uint8_t value);
  
  /* Print debug message */
  void debug_msg(char* s, uint32_t value);
  
  /* Print memory dump */
  void print_dump(void* address, uint32_t size);
  
  void set_video_vaddr(void* vaddr);
  
  /* Convert decimal memory in string */
  void dec2dec_str(uint32_t value, char* dec_str);
  void dec2hex_str(uint64_t value, char* hex_str);

  /* Print text in virtual screen */
  void vprint_text(vscreen_t* vscr, char* s);

  /* Get new virtual screen */
  vscreen_t* get_vscreen(void);

  /* Destroy virtual screen */
  void destroy_vscreen(vscreen_t* vscr);

#endif
