#pragma once
#include "../../Utility/Interface.h"
#include "../../Utility/signatures.h"
#include "IAppSystem.h"

MAKE_SIG(Key_NameForBinding, "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F1 48 8D 2D", ENGINE_DLL,
	const char*, const char*);

class IInputSystem : public IAppSystem
{
public:
	// Attach, detach input system from a particular window
	// This window should be the root window for the application
	// Only 1 window should be attached at any given time.
	virtual void AttachToWindow(void* hWnd) = 0;
	virtual void DetachFromWindow() = 0;

	// Enables/disables input. PollInputState will not update current 
	// button/analog states when it is called if the system is disabled.
	virtual void EnableInput(bool bEnable) = 0;

	// Enables/disables the windows message pump. PollInputState will not
	// Peek/Dispatch messages if this is disabled
	virtual void EnableMessagePump(bool bEnable) = 0;

	// Polls the current input state
	virtual void PollInputState() = 0;

	// Gets the time of the last polling in ms
	virtual int GetPollTick() const = 0;

	// Is a button down? "Buttons" are binary-state input devices (mouse buttons, keyboard keys)
	virtual bool IsButtonDown(int code) const = 0;

	// Returns the tick at which the button was pressed and released
	virtual int GetButtonPressedTick(int code) const = 0;
	virtual int GetButtonReleasedTick(int code) const = 0;

	// Gets the value of an analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogValue(int code) const = 0;

	// Gets the change in a particular analog input device this frame
	// Includes joysticks, mousewheel, mouse
	virtual int GetAnalogDelta(int code) const = 0;

	// Returns the input events since the last poll
	virtual int GetEventCount() const = 0;
	virtual const int* GetEventData() const = 0;

	// Posts a user-defined event into the event queue; this is expected
	// to be called in overridden wndprocs connected to the root panel.
	virtual void PostUserEvent(const int& event) = 0;

	// Returns the number of joysticks
	virtual int GetJoystickCount() const = 0;

	// Enable/disable joystick, it has perf costs
	virtual void EnableJoystickInput(int nJoystick, bool bEnable) = 0;

	// Enable/disable diagonal joystick POV (simultaneous POV buttons down)
	virtual void EnableJoystickDiagonalPOV(int nJoystick, bool bEnable) = 0;

	// Sample the joystick and append events to the input queue
	virtual void SampleDevices(void) = 0;

	// FIXME: Currently force-feedback is only supported on the Xbox 360
	virtual void SetRumble(float fLeftMotor, float fRightMotor, int userId) = 0;
	virtual void StopRumble(void) = 0;

	// Resets the input state
	virtual void ResetInputState() = 0;

	// Sets a player as the primary user - all other controllers will be ignored.
	virtual void SetPrimaryUserId(int userId) = 0;

	// Convert back + forth between ButtonCode/AnalogCode + strings
	virtual const char* ButtonCodeToString(int code) const = 0;
	virtual const char* AnalogCodeToString(int code) const = 0;
	virtual int StringToButtonCode(const char* pString) const = 0;
	virtual int StringToAnalogCode(const char* pString) const = 0;

	// Sleeps until input happens. Pass a negative number to sleep infinitely
	virtual void SleepUntilInput(int nMaxSleepTimeMS = -1) = 0;

	// Convert back + forth between virtual codes + button codes
	// FIXME: This is a temporary piece of code
	virtual int VirtualKeyToButtonCode(int nVirtualKey) const = 0;
	virtual int ButtonCodeToVirtualKey(int code) const = 0;
	virtual int ScanCodeToButtonCode(int lParam) const = 0;

	// How many times have we called PollInputState?
	virtual int GetPollCount() const = 0;

	// Sets the cursor position
	virtual void SetCursorPosition(int x, int y) = 0;

	// NVNT get address to haptics interface
	virtual void* GetHapticsInterfaceAddress() const = 0;

	virtual void SetNovintPure(bool bPure) = 0;

	// read and clear accumulated raw input values
	virtual bool GetRawMouseAccumulators(int& accumX, int& accumY) = 0;

	// tell the input system that we're not a game, we're console text mode.
	// this is used for dedicated servers to not initialize joystick system.
	// this needs to be called before CInputSystem::Init (e.g. in PreInit of
	// some system) if you want ot prevent the joystick system from ever
	// being initialized.
	virtual void SetConsoleTextMode(bool bConsoleTextMode) = 0;

	virtual int* SteamControllerInterface() = 0;
	virtual int GetNumSteamControllersConnected() = 0;
	virtual bool IsSteamControllerActive() = 0;
	virtual bool IsSteamControllerConnected() = 0;
	virtual int GetSteamControllerIndexForSlot(int nSlot) = 0;
	virtual bool GetRadialMenuStickValues(int nSlot, float& fX, float& fY) = 0;
	virtual void ActivateSteamControllerActionSetForSlot(int nSlot, int eActionSet) = 0;
	virtual int GetActionSetHandle(int eActionSet) = 0;
	virtual int GetActionSetHandle(const char* szActionSet) = 0;

	// Gets the action origin (i.e. which physical input) maps to the given action for the given action set
	virtual int GetSteamControllerActionOrigin() = 0;
	virtual int GetSteamControllerActionOrigin(const char* action, int action_set_handle) = 0;

	// Maps a Steam Controller action origin to a string (consisting of a single character) in our SC icon font
	virtual const wchar_t* GetSteamControllerFontCharacterForActionOrigin(int origin) = 0;

	// Maps a Steam Controller action origin to a short text string (e.g. "X", "LB", "LDOWN") describing the control.
	// Prefer to actually use the icon font wherever possible.
	virtual const wchar_t* GetSteamControllerDescriptionForActionOrigin(int origin) = 0;

	// This is called with "true" by dedicated server initialization (before calling Init) in order to
	// force us to skip initialization of Steam (which messes up dedicated servers).
	virtual void SetSkipControllerInitialization(bool bSkip) = 0;

	// Helper - activate same action set for all controller slots.
	void ActivateSteamControllerActionSet(int eActionSet) {
		ActivateSteamControllerActionSetForSlot(0xffffffffffffffff, eActionSet);
	}

public:
	int32_t VirtualKeyForInput(const char* szInput)
	{
		// Input -> Key Name -> Button Code -> Virtual Key Code
		return ButtonCodeToVirtualKey(StringToButtonCode(Sig::Key_NameForBinding(szInput)));
	}
};

MAKE_INTERFACE_VERSION(iInputSystem, "InputSystemVersion001", IInputSystem, INPUTSYSTEM_DLL)