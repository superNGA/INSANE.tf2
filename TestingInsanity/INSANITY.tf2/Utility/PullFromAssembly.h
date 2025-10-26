#pragma once
#include <vector>
#include "ConsoleLogging.h"
#include "ModuleNames.h"

// Signature Scanning
#include "../Libraries/Utility/Utility.h"
extern Utility util;


class IASMData_t;
//class ASMData_t;

class IASMData_t
{
public:
    IASMData_t(
        const char* szSignature,
        int iStartOffset,
        const char* szName,
        const char* szModuleName,
        int iLen = 4,
        int iNextInstructionOffset = 0) :
        m_szSignature(szSignature), m_szName(szName),
        m_iStartOffset(iStartOffset),
        m_szModuleName(szModuleName), m_iLen(iLen),
        m_iNextInstructionOffest(iNextInstructionOffset)
    {
    }
    virtual bool Initialize() = 0;

    const char* m_szSignature = "NULL";
    const char* m_szName = "NULL";
    const char* m_szModuleName = "NULL";
    int m_iStartOffset = 0;
    int m_iLen = 4;
    int m_iNextInstructionOffest = 0;
};

class AllASMData_t
{
public:
    bool Initialize()
    {
        for (auto* pASMData : m_vecAllASMData)
        {
            if (pASMData->Initialize() == false)
            {
                FAIL_LOG("Failed to get [ %s ] from [ %s ]", pASMData->m_szName, pASMData->m_szSignature);
                return false;
            }

            WIN_LOG("Successfully got [ %s ] from [ %s ]", pASMData->m_szName, pASMData->m_szSignature);
        }

        WIN_LOG("Successfully got all %d objets", m_vecAllASMData.size());
        return true;
    }
    void AddToAllAsmData(IASMData_t* pData) { m_vecAllASMData.push_back(pData); }

private:
    std::vector<IASMData_t*> m_vecAllASMData = {};
};
inline AllASMData_t allASMData;


template <typename Type>
class ASMData_t : public IASMData_t
{
public:
    ASMData_t(Type** pOutput,
        const char* szSignature,
        int iStartOffset,
        const char* szName,
        const char* szModuleName,
        int iLen = 4,
        int iNextInstructionOffset = 0) :
        IASMData_t(szSignature, iStartOffset, szName, szModuleName, iLen, iNextInstructionOffset)
    {
        m_pOutput = pOutput;
        allASMData.AddToAllAsmData(this);
    }

    bool Initialize();

private:
    Type** m_pOutput = nullptr;
};

#define GET_ADRS_FROM_ASSEMBLY(name, type, signature, startOffset, iLen, Dll)\
namespace ASM{\
    inline type* name = nullptr;\
}\
namespace ASMTemp{\
    inline ASMData_t<type> temp##name(&ASM::name, signature, startOffset, #name, Dll, iLen, 0);\
}

#define GET_RIP_ADRS_FROM_ASSEMBLY(name, type, signature, Dll, startOffset, iLen, iNextInstructionOffset)\
namespace ASM {\
        inline type* name = nullptr; \
}\
namespace ASMTemp {\
    inline ASMData_t<type> temp##name(&ASM::name, signature, startOffset, #name, Dll, iLen, iNextInstructionOffset);\
}


//=========================================================================
//                     Definitions
//=========================================================================
template <typename Type>
bool ASMData_t<Type>::Initialize()
{
    uintptr_t pTarget = util.FindPattern(m_szSignature, m_szModuleName);
    if (pTarget == NULL)
        return false;

    // if RIP instruction or not?
    bool bIsRipInstruction = (m_iNextInstructionOffest != 0);
    if (bIsRipInstruction == false)
    {
        *m_pOutput = reinterpret_cast<Type*>(pTarget + m_iStartOffset);
        return true;
    }
    else // If RIP intruction then add rip offset to next instructions adrs.
    {
        int32_t iRipOffset = *reinterpret_cast<int32_t*>(pTarget + static_cast<uintptr_t>(m_iStartOffset));
        *m_pOutput         =  reinterpret_cast<Type*>(static_cast<uintptr_t>(iRipOffset) + (pTarget + static_cast<uintptr_t>(m_iNextInstructionOffest)));
        return true;
    }

    return false;
}