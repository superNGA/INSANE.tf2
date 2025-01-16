#include "thread2.h"

void execute_thread2(HINSTANCE instance)
{
	/* wait till thread 1 gets all the INTERFACES and UI is initialized*/
	while (!thread_termination_status::thread1_primed)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(100));

		#ifdef _DEBUG
		cons.Log("Waiting for thread1 to prime", FG_YELLOW);
		#endif
	}

	while (!directX::UI::UI_has_been_shutdown)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(10)); // This thread must sleep for this much in each iteration.

		/* check if in game */
		if (!interface_tf2::engine->IsInGame()) {
			std::this_thread::sleep_for(std::chrono::seconds(1));
			continue;
		}

		/* local player */
		I_client_entity* local_player = interface_tf2::entity_list->GetClientEntity(interface_tf2::engine->GetLocalPlayer());
		if (!local_player) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		netvar.local_player = (uintptr_t)local_player;

		/* get active weapon */
		entities::local::active_weapon = interface_tf2::entity_list->GetClientEntity(local_player->get_active_weapon_handle());
		if (!entities::local::active_weapon) {
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}

		global::entities_popullated = true;
	}

	#ifdef _DEBUG
	cons.Log("Exited entity thread", FG_GREEN);
	#endif

	ExitThread(0);
	thread_termination_status::thread2 = true;
	return;
}