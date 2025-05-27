#include "FeatureHandler.h"
#include <assert.h>
#include <algorithm>
#include "../Utility/ConsoleLogging.h"
#include "../Libraries/Utility/Utility.h"

#define DEBUG_FEATURE_HANDLER

//=========================================================================
//                     Tab_t methods
//=========================================================================
Tab_t::Tab_t(std::string szTabDisplayName, int iIndex)
{
    m_szTabDisplayName = szTabDisplayName;
    m_iIndex           = iIndex;

    featureHandler.RegisterTab(this);
}

void Tab_t::DumpNSort()
{
    for (const auto& [szSectionName, pSection] : m_mapSections)
    {
        m_vecSections.push_back(pSection);
        pSection->DumpNSort();
    }
    
    std::sort(m_vecSections.begin(), m_vecSections.end(), [](Section_t* a, Section_t* b) {return a->m_iIndex < b->m_iIndex; });

#ifdef  DEBUG_FEATURE_HANDLER
    for (const auto* section : m_vecSections)
        LOG("Section detected [ %s->%s ] @ %d", section->m_szTabName.c_str(), section->m_szSectionDisplayName.c_str(), section->m_iIndex);
#endif //  DEBUG_FEATURE_HANDLER
}

//=========================================================================
//                     Section_t methods
//=========================================================================
Section_t::Section_t(std::string szSectionDisplayName, std::string szTabName, int iIndex)
{
    m_szSectionDisplayName = szSectionDisplayName;
    m_szTabName            = szTabName;
    m_iIndex               = iIndex;

    featureHandler.RegisterSection(this);
}

void Section_t::DumpNSort()
{
    std::sort(m_vecFeatures.begin(), m_vecFeatures.end(), [](IFeature* a, IFeature* b) {return a->m_iIndex < b->m_iIndex; });

#ifdef  DEBUG_FEATURE_HANDLER
    for (const auto* feature : m_vecFeatures)
        LOG("feature detected [ %s->%s->%s ] @ %d [ Type : %d ]", 
            feature->m_szTabName.c_str(), feature->m_szSectionName.c_str(), feature->m_szFeatureDisplayName.c_str(), 
            feature->m_iIndex, feature->m_iDataType);
#endif //  DEBUG_FEATURE_HANDLER

}

//=========================================================================
//                     IFeature methods
//=========================================================================
IFeature::IFeature(std::string szFeatureDisplayName, std::string szSectionName, std::string szTabName, int iIndex, int32_t iKey, int iFlags, std::string szToolTip)
{
    m_szFeatureDisplayName = szFeatureDisplayName;
    m_szSectionName        = szSectionName;
    m_szTabName            = szTabName;
    m_iIndex               = iIndex;
    m_iFlags               = iFlags;
    m_iKey                 = iKey;
    m_szToolTip            = szToolTip;

    // Managing Hold only & Toggle only features
    
    // WARN if both HoldOnly and ToggleOnly flag is enabled
    if ((m_iFlags & FeatureFlag_HoldOnlyKeyBind) == true && (m_iFlags & FeatureFlag_ToggleOnlyKeyBind) == true)
    {
        FAIL_LOG("DUMB ASS NIGGA!, FEATURE [ %s ] HAS BOTH HOLD ONLY AND TOGGLE ONLY FLAG!!!!", m_szFeatureDisplayName.c_str());
    }
    // if Toggle only, then set the flag ahead of time! ( cause the flag won't be changing )
    else if (m_iFlags & FeatureFlag_ToggleOnlyKeyBind)
    {
        m_iOverrideType = OverrideType::OVERRIDE_TOGGLE;
    }
    // if Hold only, then set the flag ahead of time! ( cause the flag won't be changing )
    else
    {
        m_iOverrideType = OverrideType::OVERRIDE_HOLD;
    }

    featureHandler.RegisterFeature(this);
}

//=========================================================================
//                     AllFeature handler methods
//=========================================================================
FeatureHandler_t::FeatureHandler_t()
{
    m_bSectionRegisterationFailed = false;
    m_bTabRegisterationFailed     = false;
    m_bFeatureRegisterationFailed = false;

    m_vecRegisteredFeatures.clear();
    m_vecRegisteredSections.clear();
    m_vecRegisteredTabs.clear();

    m_vecFeatureMap.clear();
}

bool FeatureHandler_t::Initialize()
{
    // Popullating Tab's map
    for (auto* tab : m_vecRegisteredTabs)
    {
        m_mapFeatureMap.insert({ tab->m_szTabDisplayName, tab });
    }

    // Popullating Section's map
    for (auto* section : m_vecRegisteredSections)
    {
        // get tab for this section.
        auto it = m_mapFeatureMap.find(section->m_szTabName);
        
        // return false if creating bad section.
        if (it == m_mapFeatureMap.end())
        {
            FAIL_LOG("attempting to create section [ %s ] for non-existent tab [ %s ]", section->m_szSectionDisplayName.c_str(), section->m_szTabName.c_str());
            return false;
        }

        // inserting section poitner in Tab's map.
        it->second->m_mapSections.insert({ section->m_szSectionDisplayName, section });
    }

    // Popullating Feature's map
    for (auto* feature : m_vecRegisteredFeatures)
    {
        // Getting tab for this Feature.
        auto it = m_mapFeatureMap.find(feature->m_szTabName);

        // return false if creating bad Feature.
        if (it == m_mapFeatureMap.end())
        {
            FAIL_LOG("attempting to create feature [ %s ] for non-existent tab [ %s ]", feature->m_szFeatureDisplayName.c_str(), feature->m_szTabName.c_str());
            return false;
        }

        // Getting section for this feature.
        auto& mapSection = it->second->m_mapSections;
        auto section      = mapSection.find(feature->m_szSectionName);

        // Does this section exist in this tab
        if (section == mapSection.end())
        {
            FAIL_LOG("attempting to create feature [ %s ] for non-existent Section [ %s ]", feature->m_szFeatureDisplayName.c_str(), feature->m_szSectionName.c_str());
            return false;
        }

        section->second->m_vecFeatures.push_back(feature);
    }

    // Dumping - n - Sorting
    DumpNSort();
    for (auto& tab : m_vecFeatureMap)
    {
        tab->DumpNSort();
    }

    // Constructing Feature-to-Config linker map
    if (_ConstructFeatureToConfigLinkerMap() == false)
    {
        CONS.Log(FG_RED, BG_BLACK, BOLD, __FUNCTION__, false, "Failed to construct linker map for config");
        return false;
    }
    
    return true;
}

void FeatureHandler_t::RegisterTab(Tab_t* pTab)
{
    // Checking if duplicates exist.
    for (auto* tab : m_vecRegisteredTabs)
    {
        if (pTab->m_szTabDisplayName == tab->m_szTabDisplayName || tab->m_iIndex == pTab->m_iIndex)
        {
            m_bTabRegisterationFailed = true;
            FAIL_LOG("Tab ID collision between [ %s @ %d ] & [ %s @ %d ]", 
                pTab->m_szTabDisplayName.c_str(), pTab->m_iIndex,
                tab->m_szTabDisplayName.c_str(), tab->m_iIndex);
        }
    }

    // Only if no duplicates found, add tab to the list.
    if (m_bTabRegisterationFailed == false)
        m_vecRegisteredTabs.push_back(pTab);
}

void FeatureHandler_t::RegisterSection(Section_t* pSection)
{
    // Checking if duplicates exist.
    for (auto* section : m_vecRegisteredSections)
    {
        if (section->m_szTabName != pSection->m_szTabName)
            continue;

        if (section->m_szSectionDisplayName == pSection->m_szSectionDisplayName || section->m_iIndex == pSection->m_iIndex)
        {
            m_bSectionRegisterationFailed = true;
            FAIL_LOG("Section duplicate found for [ %s ]", pSection->m_szSectionDisplayName.c_str());
            break;
        }
    }

    // Only if no duplicates found, add tab to the list.
    if (m_bSectionRegisterationFailed == false)
        m_vecRegisteredSections.push_back(pSection);
}

void FeatureHandler_t::RegisterFeature(IFeature* pFeature)
{
    bool bDuplicateFound = false;

    for (auto* feature : m_vecRegisteredFeatures)
    {
        // If Tab name is same...
        if (pFeature->m_szTabName != feature->m_szTabName)
            continue;

        // If Section name is same...
        if (pFeature->m_szSectionName != feature->m_szSectionName)
            continue;

        // if either display name or index is same, then its a duplicate.
        if (pFeature->m_szFeatureDisplayName == feature->m_szFeatureDisplayName || pFeature->m_iIndex == feature->m_iIndex)
        {
            bDuplicateFound               = true;
            m_bFeatureRegisterationFailed = true;
            
            FAIL_LOG("Duplicate found for [ %s ]", pFeature->m_szFeatureDisplayName.c_str());
            
            break;
        }
    }

    if (bDuplicateFound == false)
        m_vecRegisteredFeatures.push_back(pFeature);
}

void FeatureHandler_t::DumpNSort()
{
    for (const auto& [szTabName, pTab] : m_mapFeatureMap)
    {
        m_vecFeatureMap.push_back(pTab);
    }

    std::sort(m_vecFeatureMap.begin(), m_vecFeatureMap.end(), [](Tab_t* a, Tab_t* b) {return a->m_iIndex < b->m_iIndex; });

#ifdef  DEBUG_FEATURE_HANDLER
    for (const auto* tab : m_vecFeatureMap)
        LOG("tab detected [ %s ] @ %d", tab->m_szTabDisplayName.c_str(), tab->m_iIndex);
#endif //  DEBUG_FEATURE_HANDLER
}

bool FeatureHandler_t::_ConstructFeatureToConfigLinkerMap()
{
    m_mapFeatureToConfigLinkerMap.clear();

    for (auto* tab : m_vecFeatureMap)
    {
        for (auto* section : tab->m_vecSections)
        {
            for (auto* feature : section->m_vecFeatures)
            {
                // Hash-ing Tab, Section & feature names
                FeaturePathHash_t iPathHash;
                iPathHash.m_iTabNameHash     = FNV1A32(feature->m_szTabName.c_str())     & 0xFFff;
                iPathHash.m_iSectionNameHash = FNV1A32(feature->m_szSectionName.c_str()) & 0xFFff;
                iPathHash.m_iFeatureNameHash = FNV1A32(feature->m_szFeatureDisplayName.c_str());

                // Insering into table.
                m_mapFeatureToConfigLinkerMap.insert({ std::bit_cast<int64_t>(iPathHash), feature});
            }
        }
    }

    return true;
}