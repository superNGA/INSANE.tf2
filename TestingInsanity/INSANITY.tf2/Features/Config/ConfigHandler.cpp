#include "ConfigHandler.h"
#include <fstream>
#include <filesystem>
#include <regex>

#include "../FeatureHandler.h"
#include "../../Utility/ConsoleLogging.h"

ConfigHandler_t::ConfigHandler_t()
{
    m_bRefeshConfigList = true;
    m_vecConfigFiles.clear();
}

bool ConfigHandler_t::IsFileNameAvialable(const std::string& szFileName) const
{
    std::string szAssertedFileName = _AssertFileName(szFileName);
    LOG("File [ %s ] -> %d", szAssertedFileName.c_str(), std::ifstream(szAssertedFileName).good());
    return std::ifstream(szAssertedFileName).good() == false;
}

bool ConfigHandler_t::CreateConfigFile(const std::string& szFileName)
{
    // Creating file
    std::ofstream pFile(_AssertFileName(szFileName));
    
    // failed?
    if (pFile.is_open() == false)
        return false;

    // Sign!
    _SignFile(pFile);

    pFile.close();

    m_bRefeshConfigList = true;
    return true;
}

bool ConfigHandler_t::DeleteConfigFile(const std::string& szFileName)
{
    std::string szAssertedFileName = _AssertFileName(szFileName);
    
    if (HasValidExtension(szFileName) == false)
    {
        FAIL_LOG("bad extensoin %s\n", szAssertedFileName.c_str());
        return false;
    }

    if (IsFileSigned(szAssertedFileName) == false)
    {
        FAIL_LOG("bad signature %s\n", szAssertedFileName.c_str());
        return false;
    }

    if (std::filesystem::remove(szAssertedFileName) == false)
        FAIL_LOG("Couldn't delete file");
    else
    {
        WIN_LOG("Delete file");
    }
    m_bRefeshConfigList = true;
    return true;
}

bool ConfigHandler_t::IsFileSigned(const std::string szFileName) const 
{
    std::ifstream pFile(_AssertFileName(szFileName));
    if (pFile.is_open() == false)
        return false;

    std::string szFirstLine;
    std::getline(pFile, szFirstLine);

    return szFirstLine == m_szSignature;
}

bool ConfigHandler_t::HasValidExtension(const std::string& szFileName) const 
{
    return szFileName.find(m_szExtension) != std::string::npos;
}


bool ConfigHandler_t::ReadConfigFile(std::string szFileName) const
{
    std::string szSource = _AssertFileName(szFileName);
    
    if (HasValidExtension(szSource) == false)
        return false;
    
    if (IsFileSigned(szSource) == false)
        return false;

    std::ifstream pSourceFile(szSource);
    if (pSourceFile.good() == false || pSourceFile.is_open() == false)
        return false;

    auto& mapFeatureToConfigLinker = featureHandler.GetConfigLinkerMap();
    
    std::string line;
    std::getline(pSourceFile, line); // Skipping the first line i.e. Signature.
    while (std::getline(pSourceFile, line))
    {
        std::smatch match;

        // Getting Hash
        std::regex pattern(R"(\d+)");
        if (std::regex_search(line, match, pattern) == false)
        {
            FAIL_LOG("[ %s ] has invalid patten", line.c_str());
            continue;
        }

        // Searching hash in map
        uint64_t iHash = std::stoull(match[0].str());
        auto it = mapFeatureToConfigLinker.find(iHash);
        if (it == mapFeatureToConfigLinker.end())
        {
            FAIL_LOG("Hash [ %llu ] not found in map", iHash);
            continue;
        }
        IFeature* pFeature = it->second;
        WIN_LOG("Found feature [ %s->%s->%s ] for hash [ %llu ]", 
            pFeature->m_szTabName.c_str(), pFeature->m_szSectionName.c_str(), pFeature->m_szFeatureDisplayName.c_str(), iHash);

        switch (pFeature->m_iDataType)
        {
        case IFeature::DataType::DT_BOOLEAN:
        {
            Feature<bool>* pFeatureDerived = static_cast<Feature<bool>*>(pFeature);
            
            std::smatch fullPatternMatch;
            std::regex patternBool(R"(([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+).*)");
            if (std::regex_match(line, fullPatternMatch, patternBool) == false)
            {
                FAIL_LOG("[ %s ] failed boolean patten match", line.c_str());
                break;
            }

            // Reading Key
            pFeatureDerived->m_iKey = std::stol(fullPatternMatch[2].str());
            
            // Reading Override / Toggle type
            int iOverrideType = std::stoi(fullPatternMatch[3].str());
            pFeatureDerived->m_iOverrideType = static_cast<IFeature::OverrideType>( // Gotta Clamp this shit. else can cause "undefined behaviour"
                std::clamp(iOverrideType, static_cast<int>(IFeature::OverrideType::OVERRIDE_HOLD), static_cast<int>(IFeature::OverrideType::OVERRIDE_TOGGLE))
                );

            // Reading the "actual data"
            pFeatureDerived->m_Data = static_cast<bool>(std::stoi(fullPatternMatch[4].str()));

            break;
        }
        case IFeature::DataType::DT_INTSLIDER:
        {
            Feature<IntSlider_t>* pFeatureDerived = static_cast<Feature<IntSlider_t>*>(pFeature);
            
            std::smatch fullPatternMatch;
            std::regex patternInt(R"(([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+).*)");
            if (std::regex_match(line, fullPatternMatch, patternInt) == false)
            {
                FAIL_LOG("[ %s ] failed Int pattern match", line.c_str());
                break;
            }

            // Reading Key
            pFeatureDerived->m_iKey = std::stoi(fullPatternMatch[2].str());

            // Reading Override / Toggle type
            int iOverrideType = std::stoi(fullPatternMatch[3].str());
            pFeatureDerived->m_iOverrideType = static_cast<IFeature::OverrideType>( // Gotta Clamp this shit. else can cause "undefined behaviour"
                std::clamp(iOverrideType, static_cast<int>(IFeature::OverrideType::OVERRIDE_HOLD), static_cast<int>(IFeature::OverrideType::OVERRIDE_TOGGLE))
                );

            // Reading the "actual data"
            pFeatureDerived->m_Data.m_iVal         = std::stol(fullPatternMatch[4].str());
            pFeatureDerived->m_OverrideData.m_iVal = std::stol(fullPatternMatch[5].str());

            break;
        }
        case IFeature::DataType::DT_FLOATSLIDER:
        {
            Feature<FloatSlider_t>* pFeatureDerived = static_cast<Feature<FloatSlider_t>*>(pFeature);

            std::regex patternFloat(R"(([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+).*)");
            if (std::regex_match(line, match, patternFloat) == false)
            {
                FAIL_LOG("[ %s ] failed Float pattern match", line.c_str());
                break;
            }

            // Reading Key
            pFeatureDerived->m_iKey = std::stol(match[2].str());

            // Reading Override / Toggle type
            int iOverrideType = std::stoi(match[3].str());
            pFeatureDerived->m_iOverrideType = static_cast<IFeature::OverrideType>( // Gotta Clamp this shit. else can cause "undefined behaviour"
                std::clamp(iOverrideType, static_cast<int>(IFeature::OverrideType::OVERRIDE_HOLD), static_cast<int>(IFeature::OverrideType::OVERRIDE_TOGGLE))
                );

            // Reading the "actual data"
            pFeatureDerived->m_Data.m_flVal         = std::stof(match[4].str());
            pFeatureDerived->m_OverrideData.m_flVal = std::stof(match[5].str());

            break;
        }
        case IFeature::DataType::DT_COLORDATA:
        {
            Feature<ColorData_t>* pFeatureDerived = static_cast<Feature<ColorData_t>*>(pFeature);

            std::smatch fullPatternMatch;
            std::regex patternColor(R"(([-+]?\d+)\|([-+]?\d+)\|([-+]?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+)\|([-+]?\d*\.?\d+).*)");
            if (std::regex_match(line, fullPatternMatch, patternColor) == false)
            {
                FAIL_LOG("[ %s ] failed Color patten match", line.c_str());
                break;
            }

            // Reading Key
            pFeatureDerived->m_iKey = std::stol(fullPatternMatch[2].str());

            // Reading Override / Toggle type
            int iOverrideType = std::stoi(fullPatternMatch[3].str());
            pFeatureDerived->m_iOverrideType = static_cast<IFeature::OverrideType>( // Gotta Clamp this shit. else can cause "undefined behaviour"
                std::clamp(iOverrideType, static_cast<int>(IFeature::OverrideType::OVERRIDE_HOLD), static_cast<int>(IFeature::OverrideType::OVERRIDE_TOGGLE))
                );

            // Reading the "actual data"
            pFeatureDerived->m_Data.r         = std::stof(fullPatternMatch[4].str())  / 255.0f;
            pFeatureDerived->m_Data.g         = std::stof(fullPatternMatch[5].str())  / 255.0f;
            pFeatureDerived->m_Data.b         = std::stof(fullPatternMatch[6].str())  / 255.0f;
            pFeatureDerived->m_Data.a         = std::stof(fullPatternMatch[7].str())  / 255.0f;
            pFeatureDerived->m_OverrideData.r = std::stof(fullPatternMatch[8].str())  / 255.0f;
            pFeatureDerived->m_OverrideData.g = std::stof(fullPatternMatch[9].str())  / 255.0f;
            pFeatureDerived->m_OverrideData.b = std::stof(fullPatternMatch[10].str()) / 255.0f;
            pFeatureDerived->m_OverrideData.a = std::stof(fullPatternMatch[11].str()) / 255.0f;

            break;
        }
        default:
            break;
        }
    }

    return true;
}

bool ConfigHandler_t::WriteToConfigFile(std::string szFileName) const
{
    // Checking if file is valid or not
    std::string szTarget = _AssertFileName(szFileName);
    
    if (HasValidExtension(szTarget) == false)
        return false;

    if (IsFileSigned(szTarget) == false)
        return false;

    // Opening config file
    std::ofstream pFile(szTarget, std::ios_base::trunc | std::ios_base::out);
    if (pFile.good() == false || pFile.is_open() == false)
        return false;
    _SignFile(pFile);

    // Writting to file
    auto& mapFeatureToConfigLinker = featureHandler.GetConfigLinkerMap();
    for (const auto& [iHash, pFeature] : mapFeatureToConfigLinker)
    {
        pFile << iHash << cConfigBreaker;
        pFile << pFeature->m_iKey << cConfigBreaker;
        pFile << static_cast<int32_t>(pFeature->m_iOverrideType) << cConfigBreaker;
        
        switch (pFeature->m_iDataType)
        {
        case IFeature::DataType::DT_BOOLEAN:
        {
            auto* pFeatureDerived = static_cast<Feature<bool>*>(pFeature);
            pFile << static_cast<int32_t>(pFeatureDerived->m_Data) << cConfigBreaker;
            break;
        }
        case IFeature::DataType::DT_INTSLIDER:
        {
            auto* pFeatureDerived = static_cast<Feature<IntSlider_t>*>(pFeature);
            
            // Only writting slider value, min & max value of the slider is hardcoded.
            pFile << pFeatureDerived->m_Data.m_iVal << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.m_iVal << cConfigBreaker;
            break;
        }
        case IFeature::DataType::DT_FLOATSLIDER:
        {
            auto* pFeatureDerived = static_cast<Feature<FloatSlider_t>*>(pFeature);

            // Only writting slider value, min & max value of the slider is hardcoded.
            pFile << pFeatureDerived->m_Data.m_flVal << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.m_flVal << cConfigBreaker;
            break;
        }
        case IFeature::DataType::DT_COLORDATA:
        {
            auto* pFeatureDerived = static_cast<Feature<ColorData_t>*>(pFeature);
            
            pFile << pFeatureDerived->m_Data.r         * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_Data.g         * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_Data.b         * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_Data.a         * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.r * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.g * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.b * 0xFF << cConfigBreaker;
            pFile << pFeatureDerived->m_OverrideData.a * 0xFF << cConfigBreaker;
            break;
        }
        default:
            break;
        }

        pFile << pFeature->m_szTabName << "->" << pFeature->m_szSectionName << "->" << pFeature->m_szFeatureDisplayName << '\n';
    }
    
    pFile.close();
    return true;
}


const std::vector<std::string>& ConfigHandler_t::GetAllConfigFile()
{
    if (m_bRefeshConfigList == false)
        return m_vecConfigFiles;

    m_vecConfigFiles.clear();

    for (const auto& pFile : std::filesystem::directory_iterator("."))
    {
        if (pFile.is_regular_file() == false)
            continue;

        std::string szFileName = pFile.path().filename().string();
        
        // has .INSANE extension?
        if (HasValidExtension(szFileName) == false)
            continue;

        // is Signed?
        if (IsFileSigned(szFileName) == false)
            continue;

        // This must be our file, Store the name!
        m_vecConfigFiles.push_back(szFileName);
    }

    WIN_LOG("Refreshed Config file list! FOUND : %d", m_vecConfigFiles.size());
    m_bRefeshConfigList = false;
    return m_vecConfigFiles;
}

std::string ConfigHandler_t::_AssertFileName(const std::string& szFileName) const
{
    int iValidTillIndex = 0;
    
    for (char a : szFileName)
    {
        if (a == '.')
            break;
        ++iValidTillIndex;
    }

    std::string szClippedFileName = szFileName.substr(0, iValidTillIndex);
    _AddExtension(szClippedFileName);
    return szClippedFileName;
}

void ConfigHandler_t::_SignFile(std::ofstream& pFile) const 
{
    if (pFile.is_open() == false)
        return;

    pFile << m_szSignature.c_str() << '\n';
}