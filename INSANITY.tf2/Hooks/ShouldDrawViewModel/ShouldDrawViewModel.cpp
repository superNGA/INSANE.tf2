#include "ShouldDrawViewModel.h"
#include "../../Features/config.h"

hook::shouldDrawViewModel::T_shouldDrawViewModel hook::shouldDrawViewModel::O_shouldDrawViewModel = nullptr;
bool __fastcall hook::shouldDrawViewModel::H_shouldDrawViewModel() {

	bool result = O_shouldDrawViewModel();
	if (!config::view::alwaysDrawViewModel) {
		return result;
	}
	
	return true;
}