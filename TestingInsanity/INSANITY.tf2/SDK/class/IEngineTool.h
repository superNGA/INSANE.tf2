#pragma once

#include "../../Utility/Interface Handler/Interface.h"
#include "Basic Structures.h"
#include "IBaseInterface.h"

// Exposed from engine to all tools, simplest interface
class IEngineToolFramework : public IBaseInterface
{
public:
	// Input system overrides TBD
	// Something like this
	//virtual void		AddMessageHandler( int wm_message, bool (*pfnCallback)( int wm_message, int wParam, int lParam ) ) = 0;
	//virtual void		RemoveMessageHanlder( int wm_message, bool (*pfnCallbackToRemove)( int wm_message, int wParam, int lParam ) ) = 0;

	// Helpers for implementing a tool switching UI
	virtual int			GetToolCount() const = 0;
	virtual char const* GetToolName(int index) const = 0;
	virtual void		SwitchToTool(int index) = 0;

	virtual bool		IsTopmostTool(const void* sys) const = 0;

	virtual const void* GetToolSystem(int index) const = 0;
	virtual void* GetTopmostTool() = 0;

	// Take over input
	virtual void		ShowCursor(bool show) = 0;
	virtual bool		IsCursorVisible() const = 0;
};

#define VENGINETOOLFRAMEWORK_INTERFACE_VERSION	"VENGINETOOLFRAMEWORK003"

// Exposed from engine to tools via, more involved version of above
class IEngineTool : public IEngineToolFramework
{
public:
	virtual void		GetServerFactory(int& factory) = 0;
	virtual void		GetClientFactory(int& factory) = 0;

	virtual float		GetSoundDuration(const char* pszName) = 0;
	virtual bool		IsSoundStillPlaying(int guid) = 0;
	// Returns the guid of the sound
	virtual int			StartSound() = 0;

	virtual void		StopSoundByGuid(int guid) = 0;

	// Returns how long the sound is
	virtual float		GetSoundDuration(int guid) = 0;

	// Returns if the sound is looping
	virtual bool		IsLoopingSound(int guid) = 0;
	virtual void		ReloadSound(const char* pSample) = 0;
	virtual void		StopAllSounds() = 0;
	virtual float		GetMono16Samples() = 0;
	virtual void		SetAudioState(const AudioState_t& audioState) = 0;

	// Issue a console command
	virtual void		Command(char const* cmd) = 0;
	// Flush console command buffer right away
	virtual void		Execute() = 0;

	virtual char const* GetCurrentMap() = 0;
	virtual void		ChangeToMap(char const* mapname) = 0;
	virtual bool		IsMapValid(char const* mapname) = 0;

	// Method for causing engine to call client to render scene with no view model or overlays
	// See cdll_int.h for enum RenderViewInfo_t for specifying whatToRender
	virtual void		RenderView(int& view, int nFlags, int whatToRender) = 0;

	// Returns true if the player is fully connected and active in game (i.e, not still loading)
	virtual bool		IsInGame() = 0;
	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
	virtual bool		IsConnected() = 0;

	virtual int			GetMaxClients() = 0; // Tools might want to ensure single player, e.g.

	virtual bool		IsGamePaused() = 0;
	virtual void		SetGamePaused(bool paused) = 0;

	virtual float		GetTimescale() = 0; // Could do this via ConVar system, too
	virtual void		SetTimescale(float scale) = 0;

	// Real time is unscaled, but is updated once per frame
	virtual float		GetRealTime() = 0;
	virtual float		GetRealFrameTime() = 0; // unscaled

	// Get high precision timer (for profiling?)
	virtual float		Time() = 0;

	// Host time is scaled
	virtual float		HostFrameTime() = 0; // host_frametime
	virtual float		HostTime() = 0; // host_time
	virtual int			HostTick() = 0; // host_tickcount
	virtual int			HostFrameCount() = 0; // total famecount

	virtual float		ServerTime() = 0; // gpGlobals->curtime on server
	virtual float		ServerFrameTime() = 0; // gpGlobals->frametime on server
	virtual int			ServerTick() = 0; // gpGlobals->tickcount on server
	virtual float		ServerTickInterval() = 0; // tick interval on server

	virtual float		ClientTime() = 0; // gpGlobals->curtime on client
	virtual float		ClientFrameTime() = 0; // gpGlobals->frametime on client
	virtual int			ClientTick() = 0; // gpGlobals->tickcount on client

	virtual void		SetClientFrameTime(float frametime) = 0; // gpGlobals->frametime on client

	// Currently the engine doesn't like to do networking when it's paused, but if a tool changes entity state, it can be useful to force 
	//  a network update to get that state over to the client
	virtual void		ForceUpdateDuringPause() = 0;

	// Maybe through modelcache???
	virtual model_t* GetModel() = 0;
	// Get the .mdl file used by entity (if it's a cbaseanimating)
	virtual void* GetStudioModel() = 0;

	// SINGLE PLAYER/LISTEN SERVER ONLY (just matching the client .dll api for this)
	// Prints the formatted string to the notification area of the screen ( down the right hand edge
	//  numbered lines starting at position 0
	virtual void		Con_NPrintf(int pos, const char* fmt, ...) = 0;
	// SINGLE PLAYER/LISTEN SERVER ONLY(just matching the client .dll api for this)
	// Similar to Con_NPrintf, but allows specifying custom text color and duration information
	virtual void		Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) = 0;

	// Get the current game directory (hl2, tf2, hl1, cstrike, etc.)
	virtual void        GetGameDir(char* szGetGameDir, int maxlength) = 0;

	// Do we need separate rects for the 3d "viewport" vs. the tools surface??? and can we control viewports from
	virtual void		GetScreenSize(int& width, int& height) = 0;

	// GetRootPanel(VPANEL)

	// Sets the location of the main view
	virtual void		SetMainView(const vec& vecOrigin, const qangle& angles) = 0;

	// Gets the player view
	virtual bool		GetPlayerView(int& playerView, int x, int y, int w, int h) = 0;

	// From a location on the screen, figure out the vec into the world
	virtual void		CreatePickingRay(const int& viewSetup, int x, int y, vec& org, vec& forward) = 0;

	// precache methods
	virtual bool		PrecacheSound(const char* pName, bool bPreload = false) = 0;
	virtual bool		PrecacheModel(const char* pName, bool bPreload = false) = 0;

	virtual void		InstallQuitHandler(void* pvUserData) = 0;
	virtual void		TakeTGAScreenShot(char const* filename, int width, int height) = 0;
	// Even if game is paused, force networking to update to get new server state down to client
	virtual void		ForceSend() = 0;

	virtual bool		IsRecordingMovie() = 0;

	// NOTE: Params can contain file name, frame rate, output avi, output raw, and duration
	virtual void		StartMovieRecording(void* pMovieParams) = 0;
	virtual void		EndMovieRecording() = 0;
	virtual void		CancelMovieRecording() = 0;
	virtual void* GetActiveVideoRecorder() = 0;

	virtual void		StartRecordingVoiceToFile(char const* filename, char const* pPathID = 0) = 0;
	virtual void		StopRecordingVoiceToFile() = 0;
	virtual bool		IsVoiceRecording() = 0;

	// A version that simply accepts a ray (can work as a traceline or tracehull)
	virtual void		TraceRay(const int& ray, unsigned int fMask, void* pTraceFilter, void* pTrace) = 0; // client version
	virtual void		TraceRayServer(const int& ray, unsigned int fMask, void* pTraceFilter, void* pTrace) = 0;

	virtual bool		IsConsoleVisible() = 0;

	virtual int			GetPointContents(const vec& vecPosition) = 0;

	virtual int			GetActiveDLights() = 0;
	virtual int			GetLightingConditions(const vec& vecPosition, vec* pColors, int nMaxLocalLights, void* pLocalLights) = 0;

	virtual void		GetWorldToScreenMatrixForView(const int& view, void* pVMatrix) = 0;

	// Collision support
	virtual void CreatePartitionHandle() = 0;
};

MAKE_INTERFACE_VERSION(iEngineTool, "VENGINETOOL003", IEngineTool, ENGINE_DLL)


//class CEngineTool
//{
//public:
//	virtual void		ShowCursor(bool show);
//	virtual bool		IsCursorVisible() const;
//
//	// Helpers for implementing a tool switching UI
//	virtual int			GetToolCount() const;
//	virtual const char* GetToolName(int index) const;
//	virtual void		SwitchToTool(int index);
//
//	virtual bool		IsTopmostTool(const void* sys) const;
//	virtual const void* GetToolSystem(int index) const;
//	virtual void* GetTopmostTool();
//
//public:
//	// Retrieve factories from server.dll and client.dll to get at specific interfaces defined within
//	virtual void		GetServerFactory(void* factory);
//	virtual void		GetClientFactory(void* factory);
//
//	// Issue a console command
//	virtual void		Command(const char* cmd);
//	// Flush console command buffer right away
//	virtual void		Execute();
//
//	// If in a level, get name of current level
//	virtual const char* GetCurrentMap();
//	virtual void		ChangeToMap(const char* mapname);
//	virtual bool		IsMapValid(const char* mapname);
//
//	// Method for causing engine to call client to render scene with no view model or overlays
//	virtual void		RenderView(void* view, int nFlags, int nWhatToRender);
//
//	// Returns true if the player is fully connected and active in game (i.e, not still loading)
//	virtual bool		IsInGame();
//	// Returns true if the player is connected, but not necessarily active in game (could still be loading)
//	virtual bool		IsConnected();
//
//	virtual int			GetMaxClients(); // Tools might want to ensure single player, e.g.
//
//	virtual bool		IsGamePaused();
//	virtual void		SetGamePaused(bool paused);
//
//	virtual float		GetTimescale(); // Could do this via ConVar system, too
//	virtual void		SetTimescale(float scale);
//
//	// Real time is unscaled, but is updated once per frame
//	virtual float		GetRealTime();
//	virtual float		GetRealFrameTime(); // unscaled
//
//	virtual float		Time(); // Get high precision timer (for profiling?)
//
//	// Host time is scaled
//	virtual float		HostFrameTime(); // host_frametime
//	virtual float		HostTime(); // host_time
//	virtual int			HostTick(); // host_tickcount
//	virtual int			HostFrameCount(); // total famecount
//
//	virtual float		ServerTime(); // gpGlobals->curtime on server
//	virtual float		ServerFrameTime(); // gpGlobals->frametime on server
//	virtual int			ServerTick(); // gpGlobals->tickcount on server
//	virtual float		ServerTickInterval(); // tick interval on server
//
//	virtual float		ClientTime(); // gpGlobals->curtime on client
//	virtual float		ClientFrameTime(); // gpGlobals->frametime on client
//	virtual int			ClientTick(); // gpGlobals->tickcount on client
//
//	virtual void		SetClientFrameTime(float frametime); // gpGlobals->frametime on client
//
//	// Currently the engine doesn't like to do networking when it's paused, but if a tool changes entity state, it can be useful to force 
//	//  a network update to get that state over to the client
//	virtual void		ForceUpdateDuringPause();
//
//	// Maybe through modelcache???
//	virtual void* GetModel();
//	// Get the .mdl file used by entity (if it's a cbaseanimating)
//	virtual void* GetStudioModel();
//
//	// SINGLE PLAYER/LISTEN SERVER ONLY (just matching the client .dll api for this)
//	// Prints the formatted string to the notification area of the screen ( down the right hand edge
//	//  numbered lines starting at position 0
//	virtual void		Con_NPrintf(int pos, const char* fmt, ...);
//	// SINGLE PLAYER/LISTEN SERVER ONLY(just matching the client .dll api for this)
//	// Similar to Con_NPrintf, but allows specifying custom text color and duration information
//	virtual void		Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...);
//
//	// Get the current game directory (hl2, tf2, hl1, cstrike, etc.)
//	virtual void        GetGameDir(char* szGetGameDir, int maxlength);
//
//	// Do we need separate rects for the 3d "viewport" vs. the tools surface??? and can we control viewports from
//	virtual void		GetScreenSize(int& width, int& height);
//
//	virtual int			StartSound();
//
//	virtual void	StopSoundByGuid(int guid);
//	virtual bool	IsSoundStillPlaying(int guid);
//	virtual float	GetSoundDuration(int guid);
//	virtual void	ReloadSound(const char* pSample);
//	virtual void	StopAllSounds();
//	virtual void	SetAudioState(const void* audioState);
//	virtual void	SetMainView(const vec& vecOrigin, const qangle& angles);
//	virtual bool	GetPlayerView(int& playerView, int x, int y, int w, int h);
//	virtual void	CreatePickingRay(const int& viewSetup, int x, int y, vec& org, vec& forward);
//	virtual bool	IsLoopingSound(int guid);
//
//	virtual void	InstallQuitHandler(void* pvUserData);
//	virtual void	TakeTGAScreenShot(const char* filename, int width, int height);
//	// Even if game is paused, force networking to update to get new server state down to client
//	virtual void	ForceSend();
//
//	virtual bool	IsRecordingMovie();
//
//	// NOTE: Params can contain file name, frame rate, output avi, output raw, and duration
//	virtual void	StartMovieRecording(void* pMovieParams);
//	virtual void	EndMovieRecording();
//	virtual void	CancelMovieRecording();
//	virtual void* GetActiveVideoRecorder();
//
//	virtual void	StartRecordingVoiceToFile(const char* filename, const char* pPathID = 0);
//	virtual void	StopRecordingVoiceToFile();
//	virtual bool	IsVoiceRecording();
//
//	virtual void	TraceRay(void* ray, unsigned int fMask, void* pTraceFilter, void* pTrace);
//	virtual void	TraceRayServer(void* ray, unsigned int fMask, void* pTraceFilter, void* pTrace);
//
//	bool			CanQuit();
//	void			UpdateScreenshot();
//
//	bool			ShouldSuppressDeInit() const;
//
//	virtual bool		IsConsoleVisible();
//	virtual int			GetPointContents(const vec& vecPosition);
//	virtual int			GetActiveDLights();
//	virtual int			GetLightingConditions();
//
//	// precache methods
//	virtual bool		PrecacheSound(const char* pName, bool bPreload = false);
//	virtual bool		PrecacheModel(const char* pName, bool bPreload = false);
//};