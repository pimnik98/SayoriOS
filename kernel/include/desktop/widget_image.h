#pragma once

#include <common.h>
#include "widget.h"
#include "fmt/tga.h"

typedef enum {
	NORMAL,
	STRETCH
} Widget_Image_Display_Mode_t;

typedef struct Widget_Image {
	const char* path;

	tga_header_t meta;
	void*  image_data;

    bool loaded;

	Widget_Image_Display_Mode_t display_mode;
} Widget_Image_t;

Widget_t* new_widget_image(const char *path);