//=========================================================================
//                                 CONFIG SYSTEM
//=========================================================================
// by      : INSANE
// created : 02/25/2025
// 
// purpose : create, save, modify & check config files efficiently

#include "config.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>

config_t config; // global config object, this is the config that will be used as the active config
configManager_t g_configManager; // global object for configManger

//=========================================================================
//                             PUBLIC METHODS
//=========================================================================

//=========================================================================
// bool configManager_t::createFile(std::string& fileName, fileCreationPrivilage_t creationMethod)
//=========================================================================
/**
* creates a file with .INSANE extension
*
* @param fileName : name of the file without extension, but can manage with extension 
*                   by removing the extension
* @param creationMethod : PVLG_CREATE_NEW will make a new file if file name is not avilable
*                         PVLG_OVERWRITE will delete previous file and make a new one with 
*						  that name
*/
bool configManager_t::createFile(std::string& fileName, fileCreationPrivilage_t creationMethod) {

	std::string fileName_ = fileName;
	_processName(fileName_);
	std::ifstream readFile(fileName_);

	uint16_t fileIndex = 1;
	// if file with that name already exists & we want to make a new file
	while (readFile && creationMethod == PVLG_CREATE_NEW) {

		// make new name
		fileName_ = fileName;
		fileName_ += "(";
		fileName_ += fileIndex + '0';
		fileName_ += ")";
		_processName(fileName_);

		#ifdef LOG_OUTPUT
		std::cout << "Testing name : " << fileName_ << '\n';
		#endif // LOG_OUTPUT

		fileIndex++;
		readFile = std::ifstream(fileName_);
	}

	// creating the actual file
	readFile.close();
	fileName = fileName_; // updating name
	std::ofstream writeFile(fileName, std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

	// varifying file creation
	if (!writeFile) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed file creation with name " << fileName << '\n';
		#endif
		return false;
	}
	writeFile.close();

	// signing file
	if (!_signFile(fileName)) {
		return false;
	}

	#ifdef LOG_OUTPUT
	std::cout << "created file [" << fileName << "] SUCCESSFULY\n";
	#endif 

	return true;
}

//=========================================================================
// void configManager_t::displayFile(std::string& fileName)
//=========================================================================
/**
* displays the content of the file, if console if allocated
*
* @param fileName : name of the file without extension, but can manage with extension 
*                   by removing the extension
*/
void configManager_t::displayFile(std::string& fileName) {

	_processName(fileName);
	std::ifstream readFile(fileName);
	assert(readFile && "couldn't open file when checking signature");

	if (!readFile) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed to open file " << fileName << '\n';
		#endif
		return;
	}

	readFile.seekg(std::ios_base::beg);
	std::string line_;
	while (std::getline(readFile, line_)) {
		std::cout << line_ << '\n';
	}
	readFile.close();
}

void configManager_t::update_vecAllConfigFiles() {

	vec_allConfigFiles.clear();
	for (const auto& entry : std::filesystem::directory_iterator(_directory)) {
		if (entry.is_regular_file() && entry.path().extension() == _extension) {
			vec_allConfigFiles.push_back(entry.path().filename().string());
		}
	}
}

//============================= CONFIG SAVING & LOADING MECHANISM ==========================
bool configManager_t::saveConfigToFile(std::string& fileName, config_t& config) {

	_processName(fileName);

	// if somehow file creation fails
	if (!createFile(fileName, fileCreationPrivilage_t::PVLG_OVERWRITE)) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed file creation, while trying to save config\n";
		#endif
		return false;
	}

	std::ofstream writeFile(fileName, std::ios_base::in | std::ios_base::out | std::ios_base::app);
	writeFile.seekp(std::ios_base::end);
	writeFile.write(reinterpret_cast<char*>(&config), sizeof(config));
	writeFile.close();

	#ifdef LOG_OUTPUT
	std::cout << "saved config successfuly\n";
	#endif

	return true;
}

bool configManager_t::loadConfigFromFile(std::string& fileName, config_t& config) {

	_processName(fileName);
	std::ifstream readFile(fileName);
	assert(readFile && "couldn't open file when loading config");

	if (!readFile) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed to open file " << fileName << '\n';
		#endif
		return false;
	}

	readFile.seekg(std::ios_base::beg);
	std::string line_;
	std::getline(readFile, line_);
	readFile.read(reinterpret_cast<char*>(&config), sizeof(config));
	readFile.close();

	#ifdef LOG_OUTPUT
	std::cout << "loaded config successfuly\n";
	#endif

	return true;
}

//======================== FILE SIGNATURE RELATED FNs ===============================
bool configManager_t::_signFile(std::string& fileName) {
	/* since this is an internal fn, we are not procesing name in this one.
	assumming name passed in this one is correct */

	/* is file alreay signed ? , this shouldn't happen cause we only call this in createfile and that fn
	/opens file in std::trunc, which should clear the file */
	if (isFileSigned(fileName)) {
		#ifdef LOG_OUTPUT
		std::cout << "file " << fileName << " already signed\n";
		#endif
		return false;
	}

	std::ofstream writeFile(fileName, std::ios_base::in | std::ios_base::out | std::ios_base::app);

	// did we fail to open the file
	if (!writeFile) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed to open file " << fileName << '\n';
		#endif
		return false;
	}

	writeFile.seekp(std::ios_base::beg);
	writeFile << _signature << '\n';
	writeFile.close();

	#ifdef LOG_OUTPUT
	std::cout << "Signed file " << fileName << " SUCCESSFULY\n";
	#endif

	return true;
}

bool configManager_t::isFileSigned(std::string& fileName) {

	_processName(fileName);
	std::ifstream readFile(fileName);
	assert(readFile && "couldn't open file when checking signature");

	if (!readFile) {
		#ifdef LOG_OUTPUT
		std::cout << "Failed to open file " << fileName << '\n';
		#endif
		return false;
	}

	readFile.seekg(std::ios_base::beg);
	std::string CHE_line;
	std::getline(readFile, CHE_line);
	readFile.close();

	if (CHE_line != _signature) {
		#ifdef LOG_OUTPUT
		std::cout << CHE_line << " is not a valid signature\n";
		#endif
		return false;
	}

	return true;
}

//======================= FILE NAME MANAGEMENT ==========================
void configManager_t::_stripName(std::string& fileName) {

	int16_t index = -1;
	for (char x : fileName) {
		if (index < 0) index = 0;
		if (x == '.') break;
		index++;
	}

	if (index >= 0) {
		fileName = fileName.substr(0, index);
		#ifdef LOG_OUTPUT
		std::cout << "Name already had an extension, changing it to : " << fileName << '\n';
		#endif 
	}
}

void configManager_t::_assertFileName(std::string& fileName) {

	if (fileName.length() > MAX_FILENAME_SIZE - _extension.length() - 1) {
		fileName = fileName.substr(0, MAX_FILENAME_SIZE - _extension.length());
		#ifdef LOG_OUTPUT
		std::cout << "file name was too long, changed it to : " << fileName << '\n';
		#endif
	}
}

void configManager_t::_processName(std::string& fileName) {

	_stripName(fileName);
	_assertFileName(fileName);
	fileName = fileName + std::string(DEFAULT_EXTENSION);
}