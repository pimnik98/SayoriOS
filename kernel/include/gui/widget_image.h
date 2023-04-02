#pragma once

#include <common.h>
#include <io/imaging.h>
#include "widget.h"

typedef enum Widget_Image_Display_Mode {
	NORMAL,
	STRETCH
} Widget_Image_Display_Mode_t;

typedef struct Widget_Image {
	char* path;

	struct DukeImageMeta* meta;
	void*  image_data;

	Widget_Image_Display_Mode_t display_mode;
} Widget_Image_t;

Widget_t* new_widget_image(char* path);