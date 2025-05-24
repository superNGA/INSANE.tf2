//=========================================================================
//                      INJECTOR
//=========================================================================
// by      : INSANE
// created : 06/03/2025
// 
// purpose : injects dll into process, with multiple configuration avialable
//-------------------------------------------------------------------------

#define _CRT_SECURE_NO_WARNINGS
#include "InjectorLogic.h"
injector_t injector;

#include <cstdint>
#include <TlHelp32.h>
#include <winternl.h>
#include <fstream>
#include "ConsoleSystem/ConsoleSystem.h"


//=========================================================================
//                     PUBLIC METHODS
//=========================================================================


//=========================================================================
// bool injector_t::intitialize(const char* targetProcName)
//=========================================================================
/**
* sets up process ID and other internal stuff for injector,
* returns false if process not found
*
* @param targetProcName : name of the target process
* @param dllPath : relative path to the Dll to be injected
*/
//-------------------------------------------------------------------------
bool injector_t::Intitialize(const char* targetProcName, const char* dllPath)
{
    CONS_INITIALIZE();

    // storing process name
    _targetProcessName = targetProcName;

    // checking if file exists or not
    if (GetFileAttributesA(dllPath) == INVALID_FILE_ATTRIBUTES)
    {
        LOG("Failed to find dll file");
        return false;
    }
    LOG("dll file found");

    // storing sus dll path
    strcpy(_dllPath, dllPath);
    _dllPath[strlen(dllPath)] = '\0';

    // if process name is invalid or process not found
    targetProcessID = _getProcessID(_targetProcessName);
    if (targetProcessID == 0)
    {
        LOG("PROCESS NOT FOUND NIGGAAAAAA\n\n\n\n");
        return false;
    }

    _initialized = true;
    return true;
}


//=========================================================================
// bool injector_t::inject(injectionConfig_t method);
//=========================================================================
/**
* injects dll into the process
*
* @param method : injetion method
*/
//-------------------------------------------------------------------------
bool injector_t::inject(injectionConfig_t method)
{
    if (_initialized == false)
    {
        LOG("Injector not initialized yet!!");
        return false;
    }

    switch (method)
    {
    case LOAD_LIBRARY:
        return _injectLoadLibrary();
    case MANUAL_MAP:
        return _injectManualMap();
    default:
        return false;
    }
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================


//=========================================================================
// int32_t injector_t::_getProcessID()
//=========================================================================
/**
* get the process ID for selected process
* return 0 if no process found
*
* @param procName : name of the process
*/
//-------------------------------------------------------------------------
int32_t injector_t::_getProcessID(const char* procName)
{
    uint32_t procId = 0;
    HANDLE procSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (procSnapShot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 procEntry;
    procEntry.dwSize = sizeof(procEntry);

    // failled to get the first entry in snapshot
    if (!Process32First(procSnapShot, &procEntry))
    {
        CloseHandle(procSnapShot);
        return 0;
    }

    // iterating over process from snap shot
    do
    {
        // if process name matches current process
        if (!_stricmp(procEntry.szExeFile, procName))
        {
            procId = procEntry.th32ProcessID;
            break;
        }

    } while (Process32Next(procSnapShot, &procEntry));

    // closing snap shot handle and returning process ID
    CloseHandle(procSnapShot);
    return procId;
}


// shifting the relative data by 12 bits to get the high 4 bits.
#define RELOC_FLAG32(relativeInfo) ((relativeInfo >> 12) == IMAGE_REL_BASED_HIGHLOW)
#define RELOC_FLAG64(relativeInfo) ((relativeInfo >> 12) == IMAGE_REL_BASED_DIR64)

#ifdef _WIN64
#define RELOC_FLAG RELOC_FLAG64
#else  
#define RELOC_FLAG RELOC_FLAG32
#endif


//=========================================================================
// void __stdcall ShellCode(ManualMappingData_t* pData)
//=========================================================================
/**
* gets written in target process memory & calls the dllMain from
* DLL's image
*
* @param pData : this is the base adrs of the loaded DLL's image
*/
//-------------------------------------------------------------------------
void __stdcall ShellCode(ManualMappingData_t* pData)
{
    if (pData == nullptr)
        return;

    // pData is also the base adrs for mapped DLL.
    BYTE* pBase = reinterpret_cast<BYTE*>(pData);
    IMAGE_OPTIONAL_HEADER* pDllOptinalHeader = &(reinterpret_cast<IMAGE_NT_HEADERS*>(
        pBase + reinterpret_cast<IMAGE_DOS_HEADER*>(pBase)->e_lfanew)->OptionalHeader);

    T_loadLIbraryA _LoadLibraryA    = pData->LoadLibraryA;
    T_getProcAdrs _GetProcAddress   = pData->GetProcAdrs;
    T_DllMain _DllMain              = reinterpret_cast<T_DllMain>((uintptr_t)pBase + pDllOptinalHeader->AddressOfEntryPoint);

    uintptr_t deltaBase = (uintptr_t)pBase - (uintptr_t)pDllOptinalHeader->ImageBase;
    if (deltaBase != 0)
    {
        if (pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size == 0)
            return;

        IMAGE_BASE_RELOCATION* pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
            pBase + pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
        while (pRelocData->VirtualAddress != 0)
        {
            // this is just : (sizeof block - 8)/2. image_base_relocation is 2 dwords, i.e. 8 bytes
            uint32_t nEntries = (pRelocData->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
            // the tutorial nigga is adding 1 to the pointer, which is just moving 
            // " 1 struct " ahead and thats where the pRelativeImformation is
            WORD* pRelativeInfo = reinterpret_cast<WORD*>(
                (uintptr_t)pRelocData + (sizeof(DWORD) * 2));

            for (uint32_t i = 0; i < nEntries; ++i, ++pRelativeInfo)
            {
                if (RELOC_FLAG(*pRelativeInfo) == true)
                {
                    // sum of (base + current reloc data offset) -> thats just absolute
                    // adrs for relocation data, + lower 12 bits of relative info.
                    // so this gets us the adrs of the stored adrs, we need to change
                    uintptr_t* pPatch = reinterpret_cast<uintptr_t*>(
                        pBase + pRelocData->VirtualAddress + ((*pRelativeInfo) & (0xFFF)));

                    // NOTE: that delta base can be negative and that is good
                    *pPatch = *pPatch + deltaBase;
                }
            }

            // switching to next pRelocData by adding size of block to current relocData
            // pointer
            pRelocData = reinterpret_cast<IMAGE_BASE_RELOCATION*>(
                reinterpret_cast<BYTE*>(pRelocData) + pRelocData->SizeOfBlock);
        }
    }

    //======================= fixxing imports =======================
    if (pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size != 0)
    {
        auto pDllImportDcptr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(
            pBase + pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

        while (pDllImportDcptr->Name)
        {
            char* szModulaName = reinterpret_cast<char*>(pBase + pDllImportDcptr->Name);
            HINSTANCE hDll = _LoadLibraryA(szModulaName);
            uintptr_t* pThunkRef = reinterpret_cast<uintptr_t*>(
                pBase + pDllImportDcptr->OriginalFirstThunk);
            uintptr_t* pFunkRef = reinterpret_cast<uintptr_t*>(
                pBase + pDllImportDcptr->FirstThunk);

            if (pThunkRef == nullptr)
                pThunkRef = pFunkRef;

            for (; *pThunkRef; ++pThunkRef, ++pFunkRef)
            {
                if (IMAGE_SNAP_BY_ORDINAL(*pThunkRef))
                {
                    // here we are passing the lower 2 bytes as name, cause they might have 
                    // the fucking name in them
                    *pFunkRef = _GetProcAddress(hDll, reinterpret_cast<char*>(*pThunkRef & 0XFFFF));
                }
                else
                {
                    // here were are casting some shit to a fn and getting name that way
                    IMAGE_IMPORT_BY_NAME* pImport =
                        reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(pBase + (*pThunkRef));
                    *pFunkRef = _GetProcAddress(hDll, pImport->Name);
                }
            }
            ++pDllImportDcptr;
        }
    }
    // fixxing imports done

    if (pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].Size != 0)
    {
        IMAGE_TLS_DIRECTORY* pTLS = reinterpret_cast<IMAGE_TLS_DIRECTORY*>(
            pBase + pDllOptinalHeader->DataDirectory[IMAGE_DIRECTORY_ENTRY_TLS].VirtualAddress);
        auto* pCallBack = reinterpret_cast<PIMAGE_TLS_CALLBACK*>(pTLS->AddressOfCallBacks);
        for (; pCallBack && !pCallBack; ++pCallBack)
        {
            (*pCallBack)(pBase, DLL_PROCESS_ATTACH, nullptr);
        }
    }

    _DllMain(pBase, DLL_PROCESS_ATTACH, (void*)pData);

    pData->hModule = reinterpret_cast<HINSTANCE>(pBase);
}


//=========================================================================
// bool injector_t::_injectLoadLibrary()
//=========================================================================
/**
* injects into target process via loadLibrary
*/
//-------------------------------------------------------------------------
bool injector_t::_injectLoadLibrary()
{
    // getting target process handle
    HANDLE targetprocHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, injector_t::targetProcessID);
    if (targetprocHandle == INVALID_HANDLE_VALUE)
    {
        CONS_LOG_ERROR("Failed to open dll");
        return false;
    }

    // allocating memory to target process
    void* pAllocatedMemory = VirtualAllocEx(targetprocHandle, 0, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    if (pAllocatedMemory == nullptr)
    {
        LOG("Failed to allocated memory to target process");
        return false;
    }
    if (WriteProcessMemory(targetprocHandle, pAllocatedMemory, _dllPath, strlen(_dllPath) + 1, 0) == false)
    {
        LOG("Failed to write dll path in process memory");
        return false;
    }

    // creating thread 
    HANDLE dllLoaderThread = CreateRemoteThread(targetprocHandle, 0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibraryA), pAllocatedMemory, 0, 0);
    if (dllLoaderThread == INVALID_HANDLE_VALUE || dllLoaderThread == 0)
    {
        LOG("Failed to create remote thread");
        return false;
    }
    CloseHandle(dllLoaderThread);

    return true;
}


//=========================================================================
// bool injector_t::_injectManualMap()
//=========================================================================
/**
* Injects dll into target process via manual mapping
*/
//-------------------------------------------------------------------------
bool injector_t::_injectManualMap()
{
    //----------------------- LOADING DLL IMAGE -----------------------
    std::ifstream susDllHandle(_dllPath, std::ios_base::binary | std::ios_base::ate);
    if (susDllHandle.is_open() == false)
    {
        CONS_LOG_ERROR("Failed to open dll\n");
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Opened file handle sucessfully : %p", &susDllHandle);

    auto susDllSize = susDllHandle.tellg();
    if (susDllSize < 0x1000)
    {
        susDllHandle.close();
        CONS_FASTLOG(FG_RED, " dll size [ %d ] < 0x1000. i.e. less than 1 page. INVALID DLL", susDllSize);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Detected dll of size : %d Bytes (or) %d KB", (int32_t)(susDllSize), (int32_t)(susDllSize) / 1024);

    BYTE* pDllImage = (BYTE*)malloc((uintptr_t)susDllSize);
    if (pDllImage == nullptr)
    {
        susDllHandle.close();
        CONS_LOG_ERROR("Malloc for storing dll image has failed");
        return false;
    }

    susDllHandle.seekg(0, std::ios_base::beg);
    susDllHandle.read((char*)pDllImage, (int32_t)susDllSize);
    susDllHandle.close();
    CONS_FASTLOG(FG_GREEN, "successfully stored dll image in malloc-ed array @ %p", pDllImage);
    //----------------------- Done loading dll image into memory -----------------------

    //----------------------- Checking loaded dll -----------------------
    IMAGE_DOS_HEADER* pDllHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(pDllImage);
    if (pDllHeader->e_magic != 0x5A4D) // 0x5A4D = "MZ"
    {
        CONS_LOG_ERROR("Dll format is INVALID");
        free(pDllImage);
        return false;
    }

    // For some reason the e_lfanew ( i.e. offset for NT header ) is comming out to be 0xF8
    // and it seems to be working nicely with that. IDK WTFs going on LMFAO.
#ifdef _WIN64
    IMAGE_NT_HEADERS64* pDllNTHeader = reinterpret_cast<IMAGE_NT_HEADERS64*>((uintptr_t)pDllImage + (DWORD)pDllHeader->e_lfanew);
#else
    IMAGE_NT_HEADERS32* pDllNTHeader = reinterpret_cast<IMAGE_NT_HEADERS32*>((uintptr_t)pDllImage + (uintptr_t)pDllHeader->e_lfanew);
#endif

    IMAGE_OPTIONAL_HEADER* pDllOptionalHeader = &pDllNTHeader->OptionalHeader;    // OPTINAL headers
    IMAGE_FILE_HEADER* pDllFileHeader = &pDllNTHeader->FileHeader;        // FILE headers

#ifdef _WIN64
    if (pDllFileHeader->Machine != IMAGE_FILE_MACHINE_AMD64)
    {
        CONS_LOG_ERROR("dll file is not for x64 machines");
        free(pDllImage);
        return false;
    }
#else
    if (pDllFileHeader->Machine != IMAGE_FILE_MACHINE_I386)
    {
        CONS_LOG_ERROR("dll file is not for x32 machines");
        free(pDllImage);
        return false;
    }
#endif // _WIN64
    CONS_FASTLOG(FG_GREEN, "DOS headers         : %p", pDllHeader);
    CONS_FASTLOG(FG_GREEN, "NT headers          : %p", pDllNTHeader);
    CONS_FASTLOG(FG_GREEN, "OPTIONAL headers    : %p", pDllOptionalHeader);
    CONS_FASTLOG(FG_GREEN, "FILE headers        : %p", pDllFileHeader);
    CONS_LOG_SUCCESS("DLL file loaded is valided successfully. File is most likely correct");
    //----------------------- Validated loaded dll -----------------------

    //----------------------- Allocating memory to target process -----------------------
    HANDLE targetprocHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, injector_t::targetProcessID); // process handle
    if (targetprocHandle == INVALID_HANDLE_VALUE || targetprocHandle == NULL)
    {
        CONS_FASTLOG(FG_RED, "Failed to open handle to target process : %lld", targetprocHandle);
        free(pDllImage);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "successfully opened targetProc. Handle : %lld", targetprocHandle);

    // this is actual adrs where memory is allocated
    BYTE* pDllImageLoaded = (BYTE*)VirtualAllocEx(targetprocHandle, (void*)pDllOptionalHeader->ImageBase, pDllOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (pDllImageLoaded == nullptr) // if prefered base address is not avialable
    {
        CONS_LOG_ERROR("can't allocated memory in target process at image base, trying somewhere else");
        // windows automatically allocate dll memory region, if this is done, then we would need to reloacte the dll headers and shit something like that.  
        pDllImageLoaded = (BYTE*)VirtualAllocEx(targetprocHandle, nullptr, pDllOptionalHeader->SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
        if (pDllImageLoaded == nullptr)
        {
            free(pDllImage);
            CONS_LOG_ERROR("Can't allocated memory anywhere in target process. likely due to lack of RAM or some other shit");
            return false;
        }
        CONS_FASTLOG(FG_GREEN, "Allocated memory in targetProc for DLL image at : %p", pDllImageLoaded);
    }
    else
    {
        CONS_LOG_SUCCESS("Allocated memory at the image base, no need to reallocated");
    }

    CONS_FASTLOG(FG_GREEN, "Image base : %p | allocated memory adrs : %p | difference : 0x%X", pDllOptionalHeader->ImageBase, pDllImageLoaded, pDllOptionalHeader->ImageBase - (uintptr_t)pDllImageLoaded);
    CONS_FASTLOG(FG_GREEN, "sucessfully allocated memory in target process");
    //----------------------- Memory allocation done -----------------------

    //----------------------- Writing image into target process -----------------------

    // here we are getting the first section in our DLL, by adding sizeof optinal Header to the starting adrs. of optinal header :)
    IMAGE_SECTION_HEADER* pSectionHeader = IMAGE_FIRST_SECTION(pDllNTHeader);

    // in order to iterate over the array of IMAGE_SECTION_HEADER you need to keep adding size of imageSectionHeader to it, which if 40 bytes.
    // WRITTING SECTIONS
    for (int i = 0; i < pDllFileHeader->NumberOfSections; ++i, pSectionHeader = (IMAGE_SECTION_HEADER*)((uintptr_t)pSectionHeader + sizeof(IMAGE_SECTION_HEADER)))
    {
        if (pSectionHeader->SizeOfRawData != 0)
        {
            if (WriteProcessMemory(targetprocHandle, pDllImageLoaded + pSectionHeader->VirtualAddress, pDllImage + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, nullptr) == false)
            {
                CONS_FASTLOG(FG_RED, "Failed to write section [ %s ] @ %p", pSectionHeader->Name, pDllImageLoaded + pSectionHeader->VirtualAddress);
                free(pDllImage);
                VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
                return false;
            }

            CONS_FASTLOG(FG_GREEN, "SuccessFully writting section [ %s ] @ offset [ 0x%X ] in alloacted memory in [ %s ] @ %p | size : %d",
                pSectionHeader->Name, pSectionHeader->VirtualAddress, _targetProcessName, (uintptr_t)pDllImageLoaded + pSectionHeader->VirtualAddress, pSectionHeader->SizeOfRawData);
        }
    }

    /* NOTE that till this point in code, only the sections are written in the allocated memory
    and nothing else and the the header of the first 0x11000 is still empty.
    From what I suspect, we need to write there some pointer so we can call some functions from there.*/

    // here we are pasting the data struct ( which holds pointer to function ) onto the 
    // pDllImage ( overWritting ) and then writting that modified pDllImage onto the 
    // begginning of the allocated memory in target process.\
    
    // WRITTING MODDED HEADER
    //memcpy(pDllImage, &data, sizeof(data));
    int32_t sizeTillNTHeaders = ((uintptr_t)IMAGE_FIRST_SECTION(pDllNTHeader) - (uintptr_t)pDllImage); // <- this gives us the size of NT headers, without sections headers
    int32_t headerSize = sizeTillNTHeaders + (pDllFileHeader->NumberOfSections * sizeof(IMAGE_SECTION_HEADER)); // total size of array of IMAGE_SECTOIN_HEADER( 40 * n ) + size till the NT header
    if (WriteProcessMemory(targetprocHandle, pDllImageLoaded, pDllImage, headerSize, nullptr) == FALSE)
    {
        CONS_FASTLOG(FG_RED, "Failed to Write Modded DLL header in target process");
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Wrote the Modded dll header in the [ %s ] SuccessFully | %d bytes written", _targetProcessName, headerSize);

    ManualMappingData_t data = { 0 };
    data.GetProcAdrs = (T_getProcAdrs)GetProcAddress(GetModuleHandleA("kernel32.dll"), "GetProcAddress");
    data.LoadLibraryA = (T_loadLIbraryA)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");

    if (WriteProcessMemory(targetprocHandle, pDllImageLoaded, &data, sizeof(data), nullptr) == FALSE)
    {
        CONS_LOG_ERROR("Failed to overWrite headers with data object");
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Successfully overWrite'ed dll headers | BYTEs written %d", sizeof(data));

    // HEADER VERIFICATOIN
    uintptr_t pLoadLib = 0;
    uintptr_t pGetProcAdrs = 0;
    ReadProcessMemory(targetprocHandle, pDllImageLoaded, &pLoadLib, 0x8, nullptr);
    ReadProcessMemory(targetprocHandle, pDllImageLoaded + 0x8, &pGetProcAdrs, 0x8, nullptr);
    if (pLoadLib != (uintptr_t)LoadLibraryA || pGetProcAdrs != (uintptr_t)GetProcAddress)
    {
        CONS_LOG_ERROR("Fn adrs. are not matching in the modded header written in the target process. The written modded header is most likely WRONG ");
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "%p = %p && %p = %p", pLoadLib, LoadLibraryA, pGetProcAdrs, GetProcAddress);
    CONS_LOG_SUCCESS("MODDED HEADER VERIFIED");
    CONS_FASTLOG(FG_GREEN, "EST. Entry Point location : %p | base adrs @ : %p", pDllImageLoaded + pDllOptionalHeader->AddressOfEntryPoint, pDllImageLoaded);
    //----------------------- Image writting done -----------------------

    // ALLOCATING MEMORY FOR SHELL CODE
    void* pShellCode = VirtualAllocEx(targetprocHandle, nullptr, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (pShellCode == nullptr)
    {
        CONS_FASTLOG(FG_RED, "Failed to allocate memory for Shell Code");
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Allcated memory for Shell Code @ %p", pShellCode);

    // Printing shellCode bytes BEFORE writting
    BYTE buffer[0x100] = { 0 };
    ReadProcessMemory(targetprocHandle, pShellCode, buffer, 0x100, nullptr);
    std::cout << "Shellcode dump:\n";
    for (int i = 0; i < 0x100; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    // WRITTING SHELL CODE
    if (WriteProcessMemory(targetprocHandle, pShellCode, ShellCode, 0x1000, nullptr) == FALSE)
    {
        CONS_FASTLOG(FG_RED, "Failed to Write shell Code @ %p", pShellCode);
        VirtualFreeEx(targetprocHandle, pShellCode, 0, MEM_RELEASE);
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        return false;
    }
    CONS_FASTLOG(FG_GREEN, "Wrote Shell Code @ [ %p ] successFully.", pShellCode);

    // Printing shellCode bytes AFTER writting
    ReadProcessMemory(targetprocHandle, pShellCode, buffer, 0x100, nullptr);
    std::cout << "Shellcode dump:\n";
    for (int i = 0; i < 0x100; i++) {
        printf("%02X ", buffer[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }

    CONS_FASTLOG(FG_YELLOW, "pShellCode : %p", pShellCode);
    CONS_FASTLOG(FG_YELLOW, "dll virtual image : %p", pDllImageLoaded);

    IMAGE_NT_HEADERS* pNtHeaders = reinterpret_cast<IMAGE_NT_HEADERS*>(pDllImage + reinterpret_cast<IMAGE_DOS_HEADER*>(pDllImage)->e_lfanew);
    auto pOptHeader = &(pNtHeaders->OptionalHeader);
    CONS_FASTLOG(FG_YELLOW, "EST. adrs of EntryPoint : %p | Offset : 0x%X", (uintptr_t)pDllImageLoaded + (uintptr_t)pOptHeader->AddressOfEntryPoint, pOptHeader->AddressOfEntryPoint);

    HANDLE hThread = 0;
    DWORD remoteThreadID = 0;
    hThread = CreateRemoteThreadEx(
        targetprocHandle,
        nullptr,
        0,
        reinterpret_cast<LPTHREAD_START_ROUTINE>(pShellCode),
        pDllImageLoaded,
        0,
        nullptr,
        &remoteThreadID);
    CONS_FASTLOG(FG_GREEN, "thread created in target process with ID : %d", remoteThreadID);
    CONS_LOG_SUCCESS("Invoked Shell Code");

    if (hThread == INVALID_HANDLE_VALUE || hThread == NULL)
    {
        CONS_LOG_ERROR("Failed to create remote thread");
        VirtualFreeEx(targetprocHandle, pDllImageLoaded, 0, MEM_RELEASE);
        VirtualFreeEx(targetprocHandle, pShellCode, 0, MEM_RELEASE);
        return false;
    }
    CloseHandle(hThread);
    CONS_LOG_SUCCESS("SuccessFully created remote thread");

    HINSTANCE hCheck = NULL;
    while (hCheck == NULL)
    {
        ManualMappingData_t tempData = { 0 };
        //int64_t tempData = 0;
        ReadProcessMemory(targetprocHandle, pDllImageLoaded, &tempData, sizeof(tempData), nullptr);
        hCheck = tempData.hModule;
        static float waitcounter = 0.0f;
        printf("Waiting to shellCode to finish executing, [ %.1f ] elapsed\n", waitcounter);
        Sleep(10);
        waitcounter += 10.0f;

        if (waitcounter >= 2000.0f)
        {
            CONS_LOG_ERROR("Taking too long, likely shellCode failed");
            break;
        }
    }

    // closing all bullshit...
    // Do not free the virtual image for loaded DLL cause then you are just unloading the DLL MF >:(
    free(pDllImage); // this is the dll image loaded in memory, and is used in some parts of the code till the end. so thats why its here.
    VirtualFreeEx(targetprocHandle, pShellCode, 0, MEM_RELEASE); // just free the ShellCode and that should be enough
    CloseHandle(targetprocHandle);
    pDllVirtualImage = pDllImageLoaded;

    CONS_LOG_SUCCESS("Executed manual map");
    return true;
}