#pragma once
#include <string>
#include <vector>

class ISignature_t
{
public:
    ISignature_t(const char* signature, const char* szDllName, const char* szSigName);
    bool Initialize();

    const char* m_szSignature = "NULL";
    uintptr_t m_ullAdrs = 0;
    const char* m_szDllName = "NULL";
    const char* m_szSigName = "NULL";
};

template <typename Output, typename... Args>
class Signature_t : public ISignature_t
{
public:
    Signature_t(const char* signature, const char* szDllName, const char* szSigName) :
        ISignature_t(signature, szDllName, szSigName) {}
    inline Output operator()(Args... args) const
    {
        return (reinterpret_cast<Output(__fastcall*)(Args...)>(m_ullAdrs))(args...);
    }
};

#define MAKE_SIG(name, signature, dll, returnType, ...) namespace Sig{inline Signature_t<returnType, __VA_ARGS__> name(signature, dll, #name);}

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