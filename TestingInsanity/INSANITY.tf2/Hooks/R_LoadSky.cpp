#include "../Features/FeatureHandler.h"

// UTILITY
#include "../Utility/Hook Handler/Hook_t.h"
#include "../Utility/ConsoleLogging.h"
#include "../Utility/Signature Handler/signatures.h"

#include "../Features/ImGui/NotificationSystem/NotificationSystem.h"

// SDK
#include "../SDK/class/IFileSystem.h"
#include "../SDK/class/CVar.h"


DEFINE_SECTION(SkyBox, "Misc", 5)

static const char* s_szDefaultSkyBoxes[128] = { "NULL" };
const size_t       MAX_SKYBOXES             = sizeof(s_szDefaultSkyBoxes) / sizeof(const char*);
DEFINE_FEATURE(SkyBox,      "SkyBox", DropDown_t,   SkyBox, Misc, 1, DropDown_t(s_szDefaultSkyBoxes, 1), FeatureFlag_None, "Custom SkyBox")
DEFINE_FEATURE(SkyBox_ZFar, "Z-Far", FloatSlider_t, SkyBox, Misc, 2, FloatSlider_t(-1.0f, -1.0f, 10000.0f))


std::vector<std::string> g_vecAllSkyBoxes;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
static inline void InitializeSkyBoxList()
{
    memset(s_szDefaultSkyBoxes, 0, sizeof(s_szDefaultSkyBoxes));

    FileFindHandle_t hFile = NULL;
    const char*      szSkyBoxName = I::fileSystem->FindFirst("materials/skybox/*.vmt", &hFile);
    std::string      szLastSkyBoxName("NULL");
    int              iSkyBoxIndex = 1; // we start from 1, cause @ index 0 there is "Null" string. ( in s_szDefaultSkyBoxes )
    g_vecAllSkyBoxes.clear();


    while (szSkyBoxName != nullptr && szSkyBoxName[0] != '\0' && szSkyBoxName[0] != '\n')
    {
        std::string szSkyBox(szSkyBoxName);

        // name must be bigger than the suffix.
        if (szSkyBox.size() < 7)
        {
            szSkyBoxName = I::fileSystem->FindNext(hFile);
            continue;
        }


        // remove suffix from skybox name.
        {
            std::vector<std::string> vecSkyBoxSuffix{ "rt.vmt", "bk.vmt", "lf.vmt", "ft.vmt", "up.vmt", "dn.vmt" };

            for (std::string& szSuffix : vecSkyBoxSuffix)
            {
                bool bSuffixMatched = true;
                size_t iSuffixSize = szSuffix.size(), iSkyBoxNameSize = szSkyBox.size();
                for (size_t i = iSuffixSize - 1LLU; i <= 0; i++)
                {
                    if (szSkyBox[i] != szSkyBox[iSkyBoxNameSize - (iSuffixSize - i)])
                    {
                        bSuffixMatched = false;
                        break;
                    }
                }

                // if this ain't the correct suffix, the move.
                if (bSuffixMatched == false)
                    continue;

                // if this is the right suffix, then remove it.
                szSkyBox[iSkyBoxNameSize - iSuffixSize] = '\0';
                break;
            }
        }

        // if this is same as last skybox name then we don't need to do anything.
        if (strcmp(szSkyBox.c_str(), szLastSkyBoxName.c_str()) != 0) // != operator overload ain't working for some reason.
        {
            if (iSkyBoxIndex >= MAX_SKYBOXES)
                break;

            g_vecAllSkyBoxes.push_back(szSkyBox);

            s_szDefaultSkyBoxes[iSkyBoxIndex] = g_vecAllSkyBoxes.back().c_str();
            szLastSkyBoxName                  = szSkyBox;
            //LOG("Registered skybox : %s", szLastSkyBoxName.c_str());

            iSkyBoxIndex++;
        }

        szSkyBoxName = I::fileSystem->FindNext(hFile);
    }

    Features::Misc::SkyBox::SkyBox.SetItems(s_szDefaultSkyBoxes, iSkyBoxIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
//MAKE_SIG(R_LoadSky, "40 53 48 81 EC ? ? ? ? 8B 05 ? ? ? ? B3", ENGINE_DLL, int)
MAKE_HOOK(R_LoadSky, "40 53 48 81 EC ? ? ? ? 8B 05 ? ? ? ? B3", __stdcall, ENGINE_DLL, int)
{
    static bool s_bSkyBoxInit = false;
    if (s_bSkyBoxInit == false)
    {
        InitializeSkyBoxList();
        s_bSkyBoxInit = true;
    }

    // Clamp SkyBox choice so we don't set some bullshit skybox accidentally. ( it is possible that skybox index is set to 
    // some bizzare value due to loading bad config or changes in skybox count on updates n stuff like that ).
    Features::Misc::SkyBox::SkyBox.m_iActiveData = std::clamp<int>(Features::Misc::SkyBox::SkyBox.GetData(), -1, Features::Misc::SkyBox::SkyBox.m_data.m_nItems);

    if(Features::Misc::SkyBox::SkyBox.GetData() > 0)
    {
        ConVar* pSvSkyBoxName = I::iCvar->FindVar("sv_skyname");
        pSvSkyBoxName->SetValue(Features::Misc::SkyBox::SkyBox.GetString());

        LOG("Set SkyBox to : %s", Features::Misc::SkyBox::SkyBox.GetString());
        Render::notificationSystem.PushBack("SkyBox : %s", Features::Misc::SkyBox::SkyBox.GetString());
    }


    return Hook::R_LoadSky::O_R_LoadSky();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
MAKE_HOOK(R_DrawSkyBox, "48 8B C4 55 53 41 54 41 55", __stdcall, ENGINE_DLL, void*, float flZFar, int iDrawFlags)
{
    static int s_iLastSkyBoxChoice = -1;
    
    int iSkyBoxChoice = Features::Misc::SkyBox::SkyBox.GetData();
    if (iSkyBoxChoice != s_iLastSkyBoxChoice)
    {
        s_iLastSkyBoxChoice = iSkyBoxChoice;

        Hook::R_LoadSky::H_R_LoadSky();
    }

    // Z-Far
    if (Features::Misc::SkyBox::SkyBox_ZFar.GetData().m_flVal > 0.0f)
    {
        flZFar = Features::Misc::SkyBox::SkyBox_ZFar.GetData().m_flVal;
    }

    // make sure bits are set for each side of the skybox.
    iDrawFlags |= 0b111111;

    return Hook::R_DrawSkyBox::O_R_DrawSkyBox(flZFar, iDrawFlags);
}