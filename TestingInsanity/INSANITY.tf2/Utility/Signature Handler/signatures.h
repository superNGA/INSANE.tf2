#pragma once
#include <string>
#include <vector>

class ISignature_t
{
public:
    ISignature_t(const char* signature, const char* szDllName, const char* szSigName, int iOffsetStart, int iOffsetEnd, bool bLEA = false);
    bool Initialize();

    const char* m_szSignature   = "NULL";
    uintptr_t   m_ullAdrs       = 0;
    const char* m_szDllName     = "NULL";
    const char* m_szSigName     = "NULL";
    bool        m_bLEA          = false;
    int         m_iOffsetStart  = 0; 
    int         m_iOffsetEnd    = 0;
};

template <typename Output, typename... Args>
class Signature_t : public ISignature_t
{
public:
    Signature_t(const char* signature, const char* szDllName, const char* szSigName, int iOffsetStart = 0, int iOffsetEnd = 0, bool bLEA = false) :
        ISignature_t(signature, szDllName, szSigName, iOffsetStart, iOffsetEnd, bLEA) {}
    inline Output operator()(Args... args) const
    {
        return (reinterpret_cast<Output(__fastcall*)(Args...)>(m_ullAdrs))(args...);
    }
};

#define MAKE_SIG(name, signature, dll, returnType, ...) namespace Sig{inline Signature_t<returnType, __VA_ARGS__> name(signature, dll, #name);}
#define MAKE_SIG_LEA(name, signature, dll, offsetStart, offsetEnd, returnType, ...) namespace Sig{inline Signature_t<returnType, __VA_ARGS__> name(signature, dll, #name, offsetStart, offsetEnd, true);}

class AllSignatures_t
{
public:
    void AddSig(ISignature_t* pSignature) { m_vecSignatures.push_back(pSignature); }
    bool Initialize();
    int GetNumberOfSignatures() { return m_vecSignatures.size(); }
    
private:
    std::vector<ISignature_t*> m_vecSignatures = {};
};
inline AllSignatures_t allSignatures;