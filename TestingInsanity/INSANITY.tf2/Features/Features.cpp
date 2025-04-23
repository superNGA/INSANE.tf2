#include "features.h"
#include <algorithm>
#include "../Utility/ConsoleLogging.h"


bool AllFeatures_t::Initialize()
{
    for (IFeature_t* pIFeature : m_vecAllFeatures)
    {
        std::string szPath      = pIFeature->m_szPath;
        std::string szBaseClass = _GetBaseClass(szPath);

        auto it = m_umBaseFeatures.find(szBaseClass);
        if (it == m_umBaseFeatures.end())
        {
            IFeature_t* pBaseFeature = new IFeature_t(szBaseClass, 0);
            m_umBaseFeatures.insert({ szBaseClass, pBaseFeature });
            
            #ifdef DEBUG_FEATURE_HANDLER
            WIN_LOG("Added new base feature [ %s ]", szBaseClass.c_str());
            #endif
        }

        IFeature_t* pBaseFeature = m_umBaseFeatures[szBaseClass];
        pBaseFeature->m_vecChildFeature.push_back(pIFeature);

        #ifdef DEBUG_FEATURE_HANDLER
        WIN_LOG("Added feature [ %s ] to [ %s ] with ID [ %d ]", pIFeature->m_szPath.c_str(), szBaseClass.c_str(), pIFeature->m_iRenderID);
        #endif
    }

    _SortFeatures();

    return true;
}


//=========================================================================
//                     PRIVATE METHODS
//=========================================================================
bool AllFeatures_t::_SortFeatures()
{
    for (auto& baseFeature : m_umBaseFeatures)
    {
        std::sort(baseFeature.second->m_vecChildFeature.begin(),
            baseFeature.second->m_vecChildFeature.end(),
            [](const IFeature_t* a, const IFeature_t* b) { return a->m_iRenderID < b->m_iRenderID; });
    }

#ifdef DEBUG_FEATURE_HANDLER
    WIN_LOG("Sorted all features!");

    for (auto& baseFeature : m_umBaseFeatures)
    {
        for (auto* childFeature : baseFeature.second->m_vecChildFeature)
            LOG("%s [ ID : %d ]", childFeature->m_szPath.c_str(), childFeature->m_iRenderID);
    }
#endif

    return true;
}


std::string AllFeatures_t::_GetBaseClass(std::string szInput)
{
    auto iPos = szInput.find(FEATURE_NAME_CONNECTOR);
    if (iPos <= 0)
        return "";
    
    return szInput.substr(0, iPos);
}


bool AllFeatures_t::_TrimBaseClass(std::string& szInput)
{
    auto iPos = szInput.find(FEATURE_NAME_CONNECTOR);
    if (iPos <= 0)
        return false;

    szInput = szInput.substr(iPos + std::string(FEATURE_NAME_CONNECTOR).length());
}


uint32_t AllFeatures_t::_GetDepth(const std::string& szPath)
{
    uint32_t iDepth = 0;
    std::string szConnector = std::string(FEATURE_NAME_CONNECTOR);
    for (int i=0; i<szPath.length(); i++)
    {
        char x = szPath[i];

        if (x == FEATURE_NAME_CONNECTOR[0])
        {
            bool bFalseMatch = false;
            for (int j = 0; j< szConnector.length(); i++)
            {
                char y = szConnector[j];
                if (y != szPath[i + j])
                {
                    bFalseMatch = true;
                    break;
                }
            }

            if (bFalseMatch == true)
                iDepth++;
        }
    }

    return iDepth;
}


std::string AllFeatures_t::_GetPathTillDepth(const uint32_t iDepthWanted, const std::string& szPath)
{
    uint32_t iDepth = 0;
    uint32_t iCutOffLength = 0;

    std::string szOutput = szPath;

    std::string szConnector = std::string(FEATURE_NAME_CONNECTOR);
    for (int i=0; i<szPath.length(); i++)
    {
        char x = szPath[i];

        if (x == FEATURE_NAME_CONNECTOR[0])
        {
            bool bFalseMatch = false;
            for (int j = 0; j< szConnector.length(); i++)
            {
                char y = szConnector[j];
                if (y != szPath[i + j])
                {
                    bFalseMatch = true;
                    break;
                }
            }

            if (bFalseMatch == true)
                iDepth++;
        }

        if (iDepth == iDepthWanted + 1)
        {
            iCutOffLength = i;
            break;
        }
    }

    if (iCutOffLength > 0)
        szOutput = szOutput.substr(0, iCutOffLength - 1); // IDK why I did '-1' but I required it :|

    return szOutput;
}