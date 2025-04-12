#pragma once
#include <string>
#include <vector>

class Signature_t
{
public:
    Signature_t(const char* signature, const char* szDllName, const char* szSigName);
    bool Initialize();

    template <typename Output, typename... Args>
    inline Output Call(Args... args)
    {
        return (reinterpret_cast<Output(__fastcall*)(Args...)>(m_ullAdrs))(args...);
    }

    const char* m_szSignature = "NULL";
    uintptr_t m_ullAdrs = 0;
    const char* m_szDllName = "NULL";
    const char* m_szSigName = "NULL";
};

#define MAKE_SIG(name, signature, dll) namespace Sig{inline Signature_t name(signature, dll, #name);}

class AllSignatures_t
{
public:
    void AddSig(Signature_t* pSignature) { m_vecSignatures.push_back(pSignature); }
    bool Initialize();
    int GetNumberOfSignatures() { return m_vecSignatures.size(); }
    
private:
    std::vector<Signature_t*> m_vecSignatures = {};
};
inline AllSignatures_t allSignatures;