#include "common.h"
#include "drv/input/keymap.h"
#include "cpu.h"
#include "io/rgb_image.h"
#include "io/screen.h"

static unsigned int frames;

static int button_start, button_select;
static int button_a, button_b;
static int button_down, button_up, button_left, button_right;
static int button_quit;

struct keymap {
	char code;
	int *key;
	void (*f)(void);
	int prev;
};

static struct keymap keys[] =
{
	{KEY_A,     &button_a,      NULL, 0},
	{KEY_S,     &button_b,      NULL, 0},
	{KEY_D,     &button_select, NULL, 0},
	{KEY_F,     &button_start,  NULL, 0},
	{KEY_LEFT,  &button_left,   NULL, 0},
	{KEY_RIGHT, &button_right,  NULL, 0},
	{KEY_UP,    &button_up,     NULL, 0},
	{KEY_DOWN,  &button_down,   NULL, 0},
	{KEY_ESC,   &button_quit,   NULL, 0}
};

#define WIDTH 640
#define HEIGHT 576

uint32_t* gb_framebuffer = 0;

static size_t offsetx = 0;
static size_t offsety = 0;

int sdl_init(void)
{
	gb_framebuffer = kcalloc(WIDTH * HEIGHT, 4);
	
	offsetx = (getScreenWidth() - WIDTH) / 2;
	offsety = (getScreenHeight() - HEIGHT) / 2;
	
	/*
	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow(
		"Fer is an ejit",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640, 576,
		SDL_WINDOW_INPUT_FOCUS
	);

	surface = SDL_GetWindowSurface(window);
	*/

	printf("SDL init\n");

	drawRect(0, 0, getScreenWidth(), getScreenHeight(), 0x00000000);
	punch();

	return 0;
}

int sdl_update(void)
{
	size_t i;

	int chr = getCharRaw() % 128;

	for(i = 0; i < sizeof (keys) / sizeof (struct keymap); i++) {
		if(keys[i].code != chr) {
			if(keys[i].key)
				*(keys[i].key) = 0;
			continue;
		}

		if(keys[i].f && keys[i].prev == 0) {
			*(keys[i].key) = 1;
			keys[i].f();
		}

		keys[i].prev = *(keys[i].key);
		*(keys[i].key) = keys[i].code == chr;
	}

	if(button_quit) {
		printf("frames: %d\n", frames);
		return 1;
	}

	return 0;
}

unsigned int sdl_get_buttons(void)
{
	return (button_start << 3) | (button_select << 2) | (button_b << 1) | button_a;
}

unsigned int sdl_get_directions(void)
{
	return (button_down << 3) | (button_up << 2) | (button_left << 1) | button_right;
}

unsigned int *sdl_get_framebuffer(void)
{
	return gb_framebuffer;
}

void gb_display_helper(uint8_t* display_addr) {
	size_t real_bpp = framebuffer_bpp >> 3;

	for(register int i = 0; i < HEIGHT; i++) {
		int iy = i + offsety;

		for(register int j = 0; j < WIDTH; j++) {			
		    uint8_t* pixels = display_addr + ((j + offsetx) * real_bpp) + iy * framebuffer_pitch;
		    uint32_t color = *(uint32_t*)(((char*)gb_framebuffer) + PIXIDX(WIDTH * 4, j * 4, i));

			pixels[0] = color & 255;
			pixels[1] = (color >> 8) & 255;
			pixels[2] = (color >> 16) & 255;
		}
	}
}

void sdl_frame(void)
{
	frames++;

	uint8_t* displ = (uint8_t*)getDisplayAddr();

	gb_display_helper(displ);

	// printf("Update\n");
}

void sdl_quit()
{
	free(gb_framebuffer);
}
