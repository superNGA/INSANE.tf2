#include "config.h"
#include <iostream>
#include <string>

config_t config;

using std::ios_base;

bool  configManager_t::initializeFile(file_t& file, overWritePreference_t preference) {

	switch (preference) {
	case FILE_OVERWRITE: return init_overWrite(file);
	case FILE_CREATENEW: return init_createNew(file);
	default: return false;
	}
}


bool  configManager_t::saveConfigToFile(file_t& file, config_t& data) {

	if (!isFileSigned(file)) return false;
	openHandle(file);

	file.IO_file.seekp(ios_base::end);
	file.IO_file.write(reinterpret_cast<char*>(&data), sizeof(data));

	file.IO_file.close();
	return true;
}


bool  configManager_t::overWriteConfigFile(std::string fileName, config_t& data) {
	confirmFileName(fileName);
	file_t TEMP_file(fileName);
	return overWriteConfigFile(TEMP_file, data);
}


bool  configManager_t::overWriteConfigFile(file_t& file, config_t& data) {

	if (!isFileSigned(file)) {
		return false;
	}

	openHandle(file, handlePreference_t::HWND_TRUNCATE);

	// does this file exist
	if (!file.IO_file) {
		if (isConsoleAllocated()) {
			std::cout << "File doesn't exist\n";
		}
		return false;
	}

	signFile(file);
	openHandle(file, handlePreference_t::HWND_APPEND);

	file.IO_file.seekp(ios_base::end);
	file.IO_file.write(reinterpret_cast<char*>(&data), sizeof(data));

	file.IO_file.close();
	return true;
}


void  configManager_t::confirmFileName(std::string& fileName) {

	uint16_t lenExt = extension.length();
	uint16_t lenFileName = fileName.length();
	int16_t startPos = lenFileName - lenExt;

	// handling obvious cases
	if (startPos <= 0) {
		printf("invalid file name found : %s | adjusted it to be : %s\n", fileName.c_str(), getFullFileName(fileName).c_str());
		fileName = getFullFileName(fileName);
		return;
	}

	printf("len sig : %d, lenFileName : %d, startPos : %d\n", lenExt, lenFileName, startPos);

	for (int i = startPos; i < lenFileName; i++) {
		if (fileName[i] != extension[i - startPos]) {
			printf("invalid file name found : %s | adjusted it to be : %s\n", fileName.c_str(), getFullFileName(fileName).c_str());
			fileName = getFullFileName(fileName);
			return;
		}
	}
}


bool configManager_t::loadFromConfigFile(std::string fileName, config_t& data) {

	confirmFileName(fileName);
	file_t TEMP_file(fileName);
	return loadFromConfigFile(TEMP_file, data);
}


bool  configManager_t::loadFromConfigFile(file_t& file, config_t& data) {

	// open file with given name in read mode
	openHandle(file, handlePreference_t::HWND_READ);

	// does this file exist
	if (!file.IO_file) {
		if (isConsoleAllocated()) {
			std::cout << "File doesn't exist\n";
		}
		return false;
	}

	// is file signed ?
	if (!isFileSigned(file)) {
		if (isConsoleAllocated()) {
			std::cout << "File not signed, invalid file\n";
		}
		return false;
	}

	// skip past the signature check
	file.IO_file.seekg(ios_base::beg);
	std::string firstLine;
	std::getline(file.IO_file, firstLine);

	// read and load that shit into config_t object
	file.IO_file.read(reinterpret_cast<char*>(&data), sizeof(data));

	file.IO_file.close();
	return true;
}


void  configManager_t::freeFile(file_t& file) {

	if (file.IO_file.is_open())	file.IO_file.close();
	if (isConsoleAllocated()) {
		printf("free'ed file [%s]\n", file.fileName.c_str());
	}
}


void  configManager_t::openHandle(file_t& file, handlePreference_t hwndPreference) {

	if (file.IO_file.is_open()) file.IO_file.close();

	switch (hwndPreference)
	{
	case HWND_TRUNCATE:
		file.IO_file = std::fstream(file.fileName.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);
		return;
	case HWND_READ:
		file.IO_file = std::fstream(file.fileName.c_str(), std::ios_base::in);
		return;
	default:
		file.IO_file = std::fstream(file.fileName.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::app);
		return;
	}
}


bool  configManager_t::signFile(file_t& file) {

	if (!file.b_isInitialized) return false;
	if (isFileSigned(file)) return true;

	openHandle(file);
	file.IO_file << signature << '\n';
	file.IO_file.close();

	return true;
}


void  configManager_t::displayContents(file_t& file) {

	if (!isConsoleAllocated()) return;
	std::ifstream tempFile(file.fileName);

	std::cout << "\n-------FILE START------\n";
	std::stringstream buffer;
	buffer << tempFile.rdbuf();  // Read entire file into buffer
	std::cout << buffer.str(); // Print everything at once
	std::cout << "\n--------FILE END--------\n\n";

	tempFile.close();
}


bool  configManager_t::isFileSigned(file_t& file) {

	openHandle(file, HWND_APPEND);
	file.IO_file.seekg(std::ios::beg); // going to the beginning

	// reading first line
	std::string CHE_line;
	std::getline(file.IO_file, CHE_line);

	if (CHE_line == signature) {
		if (isConsoleAllocated()) {
			printf("file [%s] is Signed with signature [%s]\n", file.fileName.c_str(), signature.c_str());
		}
		return true;
	}

	if (isConsoleAllocated()) {
		printf("not signed, SIG : %s\n", signature.c_str());
	}

	file.IO_file.close();
	return false;
}


bool  configManager_t::init_createNew(file_t& file) {

	std::string TEMP_fileName = getFullFileName(file.fileName);
	std::ifstream readFile(TEMP_fileName.c_str());

	// if file doesn't exist, with this name
	if (!readFile) {
		if (isConsoleAllocated()) {
			printf("No file exist with name [%s], creating NEW file\n", file.fileName.c_str());
		}
		readFile.close();
		return init_overWrite(file);
	}
	if (isConsoleAllocated()) {
		printf("File already exists with name [%s]\n", TEMP_fileName.c_str());
	}

	// if file already exists with given name, then testing different names by adding numbers at end. like test(1)...
	uint8_t duplicateFileIndex = 1;
	while (readFile) {

		if (duplicateFileIndex > 100) return false;

		// FORMATTING NAME
		const char charIndex = duplicateFileIndex + '0';
		TEMP_fileName = file.fileName;
		TEMP_fileName += "(";
		TEMP_fileName += charIndex;
		TEMP_fileName += ")";

		// FINDING FILE
		readFile = std::ifstream(std::string(TEMP_fileName + extension)); // testing with new name
		if (isConsoleAllocated()) {
			std::cout << "trying to create file : " << TEMP_fileName << '\n';
		}

		duplicateFileIndex++;
	}

	readFile.close();
	file.fileName = TEMP_fileName; // this name is without extension here
	init_overWrite(file);
	return true;
}


bool  configManager_t::init_overWrite(file_t& file) {

	//std::string TEMP_fileName = getFullFileName(file.fileName);
	file.fileName = getFullFileName(file.fileName); // adding extension to the name, once the name is finalized
	file.IO_file = std::fstream(file.fileName.c_str(), std::ios_base::in | std::ios_base::out | std::ios_base::trunc);

	// if failed to open file
	if (!file.IO_file) {
		if (isConsoleAllocated()) {
			std::cout << "FAILED TO CREATE/OPEN FILE [ " << file.fileName << " ] IN [overWrite] mode\n";
		}
		return false;
	}

	if (isConsoleAllocated()) {
		std::cout << "SUCCESSFULLY created file : " << file.fileName << '\n';
	}

	file.b_isInitialized = true;
	signFile(file);
	file.IO_file.close();
	return true;
}


std::string configManager_t::getFullFileName(std::string fileName) {
	return fileName + extension;
}


void  configManager_t::setSignature(const char* str_signature) {
	signature = std::string(str_signature);
}


bool  configManager_t::setExtension(const char* str_extension) {

	if (str_extension[0] != '.') {
		if (isConsoleAllocated()) {
			printf("Invalid extension : %s\n", str_extension);
		}
		return false;
	}

	extension = std::string(str_extension);
	return true;
}


bool configManager_t::isConsoleAllocated() {

	static bool firstCall = true;

	if (firstCall) {
		b_consoleAvailable = (GetConsoleWindow() != NULL);
		firstCall = false;
	}
	return b_consoleAvailable;
}