#pragma once

#ifdef _DEBUG
#define LOG_OUTPUT
#endif

#include <Windows.h>
#include <fstream>
#include <sstream>
#include <vector>
#include "../SDK/class/Basic Structures.h"
#include <string>

#define DEFAULT_EXTENSION ".INSANE"
#define DEFAULT_SIGNATURE "INSANE.TF2 CONFIG FILE"

#define MAX_FILENAME_SIZE 50

//======================= HELPER FOR CONFIG STRUCTS =======================
struct ChamSetting_t
{
	bool bIgnorez = false;
	bool bChams	  = false;
	clr_t clrChams;
};

//======================= CONFIG STRUCTS =======================
struct aimbotConfig_t
{
	bool global						= false; // <- ain't working from here
	float FOV						= 10.0f;
	bool projectile_aimbot			= false;
	bool future_pos_helper			= false;
	bool autoShoot					= false; // till here...
	
	bool bNoSpread					= false;
};
struct visualConfig_t
{
	bool ESP						= false; // <- not working from here...
	bool healthBar					= false;
	bool skipDisguisedSpy			= false;
	bool skipCloackedSpy			= false;
	bool playerName					= false; // till here...

//----------------------- Chams -----------------------
	// PLAYER CHAMS
	ChamSetting_t ChamEnemyPlayer;
	ChamSetting_t ChamFriendlyPlayer;

	// SENTRY CHAMS
	ChamSetting_t ChamEnemySentry;
	ChamSetting_t ChamFriendlySentry;

	// DISPENSER CHAMS
	ChamSetting_t ChamEnemyDispenser;
	ChamSetting_t ChamFriendlyDispenser;

	// TELEPORTER CHAMS
	ChamSetting_t ChamEnemyTeleporter;
	ChamSetting_t ChamFriendlyTeleporter;

	ChamSetting_t ChamDroppedAmmoPack;
	ChamSetting_t ChamMedkit;
	ChamSetting_t ChamAnimAmmoPack;
	ChamSetting_t ChamTFItem;

	ChamSetting_t ChamEnemyProjectile;
	ChamSetting_t ChamFriendlyProjectile;

	ChamSetting_t ChamViewModel;
	ChamSetting_t ChamDroppedWeapon;
};
struct viewConfig_t
{
	float FOV						= 90.0f;
	bool RemoveSniperScopeOverlay	= false;
	bool RemoveSniperChargeOverlay	= false;
	bool alwaysRenderInThirdPerson	= false;
	bool alwaysDrawViewModel		= false;
};
struct miscConfig_t
{
	bool bhop						= false;
	bool rocket_jump				= false;
	bool third_person				= false;
	bool autoBackStab				= false;
	bool airMove					= false;

	float flNoSpreadOffset			= 0.0f;
	int32_t iServerTimeOffset = 0;
};

struct config_t 
{
	aimbotConfig_t aimbotConfig;
	visualConfig_t visualConfig;
	viewConfig_t viewConfig;
	miscConfig_t miscConfig;
};
extern config_t config;

//================== FILE MANAGEMENT SYSTEM =============================

/* todo : 
	when the cheat is done most of the way, or you have the loader.exe that injects the cheat
	properly, make it so that it makes a config folder in the directory of loader and then 
	saves the further configs there. */

/* todo :
	make a capping logic, which prevents it from breaking the cheat trying to load
	tampered or corrupted config files */

class configManager_t {
public:
	enum fileCreationPrivilage_t {
		PVLG_OVERWRITE = 0,
		PVLG_CREATE_NEW
	};

	bool createFile			(std::string& fileName, fileCreationPrivilage_t creationMethod = PVLG_CREATE_NEW);
	bool isFileSigned		(std::string& fileName);
	void displayFile		(std::string& fileName);

	bool saveConfigToFile	(std::string& fileName, config_t& config);
	bool loadConfigFromFile	(std::string& fileName, config_t& config);

	void update_vecAllConfigFiles();

	std::vector<std::string> vec_allConfigFiles;
	uint16_t activeConfigIndex = 0;

private:
	bool _signFile			(std::string& fileName);

	/* these manage file name, makes sure they are up to standard */
	void _processName		(std::string& fileName);
	void _stripName			(std::string& fileName);
	void _assertFileName	(std::string& fileName);

	std::string _extension = DEFAULT_EXTENSION;
	std::string _signature = DEFAULT_SIGNATURE;
	std::string _directory = "."; // to be changed later
};
extern configManager_t g_configManager;