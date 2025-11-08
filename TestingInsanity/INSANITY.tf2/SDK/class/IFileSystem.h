#pragma once
#include "IAppSystem.h"
#include "IBaseFileSystem.h"
#include "../../Utility/Interface Handler/Interface.h"


typedef int FileFindHandle_t;


class IFileSystem : public IAppSystem, public IBaseFileSystem
{
public:
	//--------------------------------------------------------
	// Steam operations
	//--------------------------------------------------------

	virtual bool			IsSteam() const = 0;

	// Supplying an extra app id will mount this app in addition 
	// to the one specified in the environment variable "steamappid"
	// 
	// If nExtraAppId is < -1, then it will mount that app ID only.
	// (Was needed by the dedicated server b/c the "SteamAppId" env var only gets passed to steam.dll
	// at load time, so the dedicated couldn't pass it in that way).
	virtual	int MountSteamContent(int nExtraAppId = -1) = 0;

	//--------------------------------------------------------
	// Search path manipulation
	//--------------------------------------------------------

	// Add paths in priority order (mod dir, game dir, ....)
	// If one or more .pak files are in the specified directory, then they are
	//  added after the file system path
	// If the path is the relative path to a .bsp file, then any previous .bsp file 
	//  override is cleared and the current .bsp is searched for an embedded PAK file
	//  and this file becomes the highest priority search path ( i.e., it's looked at first
	//   even before the mod's file system path ).
	virtual void			AddSearchPath(const char* pPath, const char* pathID, int addType = 0) = 0;
	virtual bool			RemoveSearchPath(const char* pPath, const char* pathID = 0) = 0;

	// Remove all search paths (including write path?)
	virtual void			RemoveAllSearchPaths(void) = 0;

	// Remove search paths associated with a given pathID
	virtual void			RemoveSearchPaths(const char* szPathID) = 0;

	// This is for optimization. If you mark a path ID as "by request only", then files inside it
	// will only be accessed if the path ID is specifically requested. Otherwise, it will be ignored.
	// If there are currently no search paths with the specified path ID, then it will still
	// remember it in case you add search paths with this path ID.
	virtual void			MarkPathIDByRequestOnly(const char* pPathID, bool bRequestOnly) = 0;

	// converts a partial path into a full path
	// Prefer using the RelativePathToFullPath_safe template wrapper to calling this directly
	virtual const char* RelativePathToFullPath(const char* pFileName/*, const char* pPathID, OUT_Z_CAP(maxLenInChars) char* pDest, int maxLenInChars, PathTypeFilter_t pathFilter = FILTER_NONE, PathTypeQuery_t* pPathType = NULL*/) = 0;


	// Returns the search path, each path is separated by ;s. Returns the length of the string returned
	// Prefer using the GetSearchPath_safe template wrapper to calling this directly
	virtual int				GetSearchPath(const char* pathID, bool bGetPackFiles/*, OUT_Z_CAP(maxLenInChars) char* pDest, int maxLenInChars*/) = 0;


	// interface for custom pack files > 4Gb
	virtual bool			AddPackFile(const char* fullpath, const char* pathID) = 0;

	//--------------------------------------------------------
	// File manipulation operations
	//--------------------------------------------------------

	// Deletes a file (on the WritePath)
	virtual void			RemoveFile(char const* pRelativePath, const char* pathID = 0) = 0;

	// Renames a file (on the WritePath)
	virtual bool			RenameFile(char const* pOldPath, char const* pNewPath, const char* pathID = 0) = 0;

	// create a local directory structure
	virtual void			CreateDirHierarchy(const char* path, const char* pathID = 0) = 0;

	// File I/O and info
	virtual bool			IsDirectory(const char* pFileName, const char* pathID = 0) = 0;

	virtual void			FileTimeToString(char* pStrip, int maxCharsIncludingTerminator, long fileTime) = 0;

	//--------------------------------------------------------
	// Open file operations
	//--------------------------------------------------------

	virtual void			SetBufferSize(FileHandle_t file, unsigned nBytes) = 0;

	virtual bool			IsOk(FileHandle_t file) = 0;

	virtual bool			EndOfFile(FileHandle_t file) = 0;

	virtual char* ReadLine(char* pOutput, int maxChars, FileHandle_t file) = 0;
	virtual int				FPrintf(FileHandle_t file, const char* pFormat, ...) = 0;

	//--------------------------------------------------------
	// Dynamic library operations
	//--------------------------------------------------------

	// load/unload modules
	virtual void* LoadModule(const char* pFileName, const char* pPathID = 0, bool bValidatedDllOnly = true) = 0;
	virtual void			UnloadModule(void* pModule) = 0;

	//--------------------------------------------------------
	// File searching operations
	//--------------------------------------------------------

	// FindFirst/FindNext. Also see FindFirstEx.
	virtual const char* FindFirst(const char* pWildCard, FileFindHandle_t* pHandle) = 0;
	virtual const char* FindNext(FileFindHandle_t handle) = 0;
	virtual bool			FindIsDirectory(FileFindHandle_t handle) = 0;
	virtual void			FindClose(FileFindHandle_t handle) = 0;

	// Same as FindFirst, but you can filter by path ID, which can make it faster.
	virtual const char* FindFirstEx(
		const char* pWildCard,
		const char* pPathID,
		FileFindHandle_t* pHandle
	) = 0;
};


MAKE_INTERFACE_VERSION(fileSystem, "VFileSystem022", IFileSystem, FILESYSTEM_STDIO_DLL)