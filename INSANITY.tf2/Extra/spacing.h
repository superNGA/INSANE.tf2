#pragma once
#include "../Hooks/EndScene/EndScene.h"

namespace asthetic
{
	void left_spacing(int multiplier = 1);
	void top_spacing(int mulitplier = 1);

	/*Note : create button first them call this function,
	otherwise the alignment can get fucked up*/
	void create_button_with_image(ImVec2 cursor_pos_before_button, texture_data& image, const char* text = "", bool image_one_both_sides = false);

	void create_half_button(ImVec2 cursor_pos, texture_data& image);
};