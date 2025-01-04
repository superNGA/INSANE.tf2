#include "spacing.h"

void asthetic::create_half_button(ImVec2 cursor_pos, texture_data& image)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();
	ImVec2 image_start(
		cursor_pos.x + ((directX::UI::standard_button_size.x / 2.0f) - image.image_width) / 2.0f,
		cursor_pos.y + (directX::UI::standard_button_size.y - image.image_height) / 2.0f);
	draw_list->AddImage((ImTextureID)image.texture, image_start, ImVec2(image_start.x + image.image_width, image_start.y + image.image_height));
}


void asthetic::create_button_with_image(ImVec2 cursor_pos_before_button, texture_data& image, const char* text, bool image_one_both_sides)
{
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

	image.rendering_image_height = 25.0f;
	image.rendering_image_width = image.rendering_image_height * image.image_width / image.image_height;

	/*padding*/
	float y_padding = (directX::UI::standard_button_size.y - image.rendering_image_height) / 2;

	/* text widht */
	ImVec2 text_size = ImGui::CalcTextSize(text);

	/*measurments*/
	ImVec2 image_start(cursor_pos_before_button.x + directX::UI::padding_001.x, cursor_pos_before_button.y + y_padding);
	ImVec2 second_image_start(
		cursor_pos_before_button.x + directX::UI::standard_button_size.x - directX::UI::padding_001.x - image.rendering_image_width,
		cursor_pos_before_button.y + directX::UI::standard_button_size.y - y_padding - image.rendering_image_height);

	ImVec2 text_pos(cursor_pos_before_button.x + (directX::UI::standard_button_size.x - text_size.x) / 2, cursor_pos_before_button.y + (directX::UI::standard_button_size.y - text_size.y) / 2);

	/*drawing first image*/
	draw_list->AddImage(
		(ImTextureID)image.texture,
		image_start,
		ImVec2(image_start.x + image.rendering_image_width, image_start.y + image.rendering_image_height));

	/*drawing text*/
	draw_list->AddText(
		text_pos,
		IM_COL32(255, 255, 255, 255),
		text);
	
	/*drawing second image if told*/
	if (image_one_both_sides)
	{
		draw_list->AddImage(
			(ImTextureID)image.texture,
			second_image_start,
			ImVec2(second_image_start.x + image.rendering_image_width, second_image_start.y + image.rendering_image_height));
	}
}


void asthetic::left_spacing(int multiplier)
{
	ImGui::Dummy(ImVec2(10.0f * multiplier, 0.0f));
	ImGui::SameLine();
}


void asthetic::top_spacing(int multiplier)
{
	ImGui::Dummy(ImVec2(0.0f, 10.0f * multiplier));
}