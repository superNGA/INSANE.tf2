#pragma once
#include <Windows.h>
#include <fstream>
#include <sstream>
#include "../SDK/class/Basic Structures.h"

#define DEFAULT_EXTENSION ".INSANE"
#define SIGNATURE "INSANE CONFIG FILE"

enum featureTag {
	FTR_NOTDEFINED=-1,
	FTR_AIMBOT=0,
	FTR_VISUAL,
	FTR_VIEW,
	FTR_WORLD,
	FTR_MISC
};

struct featureData_t 
{
	featureData_t() : tag(featureTag::FTR_NOTDEFINED){}
	featureData_t(featureTag _tag) : tag(_tag){}

	float val	= 0;
	bool state	= false;
	int16_t tag = FTR_NOTDEFINED;
	char pad;
};

struct aimbotConfig_t
{
	bool global						= false;
	float FOV						= 10.0f;
	bool projectile_aimbot			= false;
	bool future_pos_helper			= false;
	bool autoShoot					= false;
};
struct visualConfig_t
{
	bool ESP						= false;
	bool healthBar					= false;
	bool skipDisguisedSpy			= false;
	bool skipCloackedSpy			= false;
	bool playerName					= false;
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
struct file_t {

	file_t(std::string _fileName) : fileName(_fileName) {}
	file_t() :fileName("config") {}
	std::fstream IO_file;
	std::string fileName = "NOT-INITIALIZED";
	bool b_isInitialized = false;
};

// how to open file when opening in write mode?
enum overWritePreference_t {
	FILE_OVERWRITE = 0, // just opens the file, overWrite if it exists
	FILE_CREATENEW // if exists then creates new with different name
};

enum handlePreference_t {
	HWND_TRUNCATE = 0,
	HWND_APPEND,
	HWND_READ
};

class configManager_t {
public:
	// INITIALIZING
	/* this is a wrapper FN for OW_openFile & CN_openFile */
	bool  initializeFile(file_t& file, overWritePreference_t preference = FILE_CREATENEW);

	// SAVING & OVERWRITTING
	bool  saveConfigToFile(file_t& file, config_t& data);
	bool  overWriteConfigFile(std::string fileName, config_t& data);
	bool  overWriteConfigFile(file_t& file, config_t& data);

	// READING / LOADING
	bool loadFromConfigFile(file_t& file, config_t& data);
	bool loadFromConfigFile(std::string fileName, config_t& data);

	void  freeFile(file_t& file);
	bool  isFileSigned(file_t& file);

	/*sets the file signature, this will be written on top of file to help us
	identify is this file is ours or not*/
	void  setSignature(const char* str_signature);

	/*give the extension type / string, INCLUDE the dot '.'*/
	bool  setExtension(const char* str_extension);

	/* prints out contents to the console. is_console_avialable() check is also done
	before printing */
	void  displayContents(file_t& file);

private:
	bool  init_overWrite(file_t& file);
	bool  init_createNew(file_t& file);
	void  openHandle(file_t& file, handlePreference_t hwndPreference = HWND_APPEND);

	/* if file name doesn't has extension properly written in it, it will add the
	extension to the file name */
	void  confirmFileName(std::string& fileName);

	bool  signFile(file_t& file);

	// add the extension to the file name and returns it
	std::string getFullFileName(std::string fileName);
	bool isConsoleAllocated();

	bool b_consoleAvailable;
	std::string extension = DEFAULT_EXTENSION;
	std::string signature = SIGNATURE;
};
inline configManager_t g_configManager;