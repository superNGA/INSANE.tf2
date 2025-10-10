//=========================================================================
//                      CONFIG HANDLER
//=========================================================================
// by      : INSANE
// created : 23/05/2025
// 
// purpose : Creates, read, write config files
//-------------------------------------------------------------------------
#pragma once

#include <string>
#include <vector>

class ConfigHandler_t
{
public:
    ConfigHandler_t();
    bool IsFileNameAvialable(const std::string& szFileName) const;
    bool CreateConfigFile(const std::string& szFileName);
    bool DeleteConfigFile(const std::string& szFileName);
    
    // Is file ours?
    bool IsFileSigned(const std::string szFileName) const;
    bool HasValidExtension(const std::string& szFileName) const;

    // Read-n-Write current settings to this file
    bool ReadConfigFile(std::string szFileName) const;
    bool WriteToConfigFile(std::string szFileName) const;

    const std::vector<std::string>& GetAllConfigFile();
    inline std::string GetExtension() { return m_szExtension; }

private:
    std::string m_szSignature   = "INSANE.TF2 CONFIG FILE";
    std::string m_szExtension   = ".INSANE";
    
    const char cConfigBreaker = '|';

    std::string _AssertFileName(const std::string& szFileName) const;
    void _SignFile(std::ofstream& pFile) const ;
    inline void _AddExtension(std::string& szFileName) const { szFileName += m_szExtension; }

    std::vector<std::string> m_vecConfigFiles = {};
    bool m_bRefeshConfigList = true;
};
inline ConfigHandler_t configHandler;