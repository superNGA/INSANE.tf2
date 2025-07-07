#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"

#include "../Features/config.h"

MAKE_HOOK(ShouldDrawViewModel, "48 83 EC ? E8 ? ? ? ? 48 85 C0 74 ? 48 8D 88 ? ? ? ? BA ? ? ? ? E8", __fastcall, CLIENT_DLL, bool, void)
{
	bool result = Hook::ShouldDrawViewModel::O_ShouldDrawViewModel();
	if (!config.viewConfig.alwaysDrawViewModel) {
		return result;
	}
	
	return true;
}