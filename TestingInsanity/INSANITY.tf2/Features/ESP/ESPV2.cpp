#include "ESPV2.h"

// SDK
#include "../../SDK/class/IVEngineClient.h"
#include "../../SDK/class/BaseEntity.h"
#include "../../SDK/class/IVDebugOverlay.h"
#include "../../SDK/class/IStudioRender.h"
#include "../../SDK/class/IVModelInfo.h"
#include "../../SDK/class/HitboxDefs.h"

// Utility
#include "../ImGui/NotificationSystem/NotificationSystem.h"
#include "../../Utility/Profiler/Profiler.h"
#include "../../Extra/math.h"
#include "../Entity Iterator/EntityIterator.h"
#include "../Graphics Engine V2/Graphics.h"
#include "../Graphics Engine V2/Draw Objects/BaseDrawObj.h"
#include "../Graphics Engine V2/Draw Objects/Box/Box.h"
#include "../Graphics Engine V2/Draw Objects/Cube/Cube.h"


/*
TODO : 
    * Skeleton ESP ( with functinality & on and off )
    * Auto stop
    * text rendering ? how difficult could it be?
    * Renderer safe deletion logic. ( IMPORTANT )
    * Entity iterator clean up.     ( IMPORTANT )
*/

constexpr size_t MAX_ESP_RECORDS = 25LLU;


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
ESP_t::ESP_t()
{
    m_vecPlayerEsp.clear();
    m_vecSentryEsp.clear();
    m_vecTeleporterEsp.clear();
    m_vecDispenserEsp.clear();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::RunCreateMove()
{
    PROFILER_RECORD_FUNCTION(CreateMove);

    if (I::iEngine->IsInGame() == false)
        return;

    if (F::entityIterator.AreListsValid() == false)
        return;


    // Sentry esp
    if(Features::Materials::Sentry::ESPSentry_Enable.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& dbEnemies = F::entityIterator.GetEnemySentry();
        std::vector<BaseEntity*>* pVecEnemies = dbEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&dbEnemies, pVecEnemies);
        _DrawEspList(m_vecSentryEsp, pVecEnemies,
            Features::Materials::Sentry::ESPSentry_RGB.GetData().m_flVal,           // RGB ? 
            Features::Materials::Sentry::ESPSentry_WidthOffset.GetData().m_flVal,   // Width offset
            Features::Materials::Sentry::ESPSentry_HeightOffset.GetData().m_flVal); // Height offset.
    }
    else
    {
        _DisableAllEsp(m_vecSentryEsp, 0LLU);
    }


    // Dispenser esp
    if(Features::Materials::Dispenser::ESPDispenser_Enable.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& dbEnemies = F::entityIterator.GetEnemyDispenser();
        std::vector<BaseEntity*>* pVecEnemies = dbEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&dbEnemies, pVecEnemies);
        _DrawEspList(m_vecDispenserEsp, pVecEnemies,
            Features::Materials::Dispenser::ESPDispenser_RGB.GetData().m_flVal,           // RGB ? 
            Features::Materials::Dispenser::ESPDispenser_WidthOffset.GetData().m_flVal,   // Width offset
            Features::Materials::Dispenser::ESPDispenser_HeightOffset.GetData().m_flVal); // Height offset.
    }
    else
    {
        _DisableAllEsp(m_vecDispenserEsp, 0LLU);
    }


    // Teleporter esp
    if(Features::Materials::Teleporter::ESPTeleporter_Enable.IsActive() == true)
    {
        Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& dbEnemies = F::entityIterator.GetEnemyTeleporter();
        std::vector<BaseEntity*>* pVecEnemies = dbEnemies.GetReadBuffer();
        DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&dbEnemies, pVecEnemies);
        _DrawEspList(m_vecTeleporterEsp, pVecEnemies,
            Features::Materials::Teleporter::ESPTeleporter_RGB.GetData().m_flVal,           // RGB ? 
            Features::Materials::Teleporter::ESPTeleporter_WidthOffset.GetData().m_flVal,   // Width offset
            Features::Materials::Teleporter::ESPTeleporter_HeightOffset.GetData().m_flVal); // Height offset.
    }
    else
    {
        _DisableAllEsp(m_vecTeleporterEsp, 0LLU);
    }

}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::RunEndScene()
{
    PROFILER_RECORD_FUNCTION(EndScene);


    // A little safety / sanity check
    if (m_vecPlayerEsp.size() > MAX_ESP_RECORDS || m_vecPlayerHitbox.size() > MAX_ESP_RECORDS)
    {
        // Scream at user to let him know that its his fault his match has more than 20 something player entities in enemy team.
        FAIL_LOG("drawing esp for more then [ %llu ] entities, something must be wrong!!!", MAX_ESP_RECORDS);
        Render::notificationSystem.PushBack("drawing esp for more then [ %llu ] entities, something must be wrong!!!", MAX_ESP_RECORDS);
        return;
    }


    // Quickly get a list of what we have to draw.
    bool bDrawEsp    = Features::Materials::Player::ESPPlayer_Enable.IsActive();
    bool bDrawHitbox = Features::Materials::Player::ESPPlayer_DrawHitbox.IsActive();


    // Don't iterate bad entitylist. It's instant crash.
    if (F::entityIterator.AreListsValid() == false)
        return;

    // Aquire and setup auto-release for enemy players.
    Containers::DoubleBuffer_t<std::vector<BaseEntity*>>& dbEnemies   = F::entityIterator.GetEnemyPlayers();
    std::vector<BaseEntity*>*                             pVecEnemies = dbEnemies.GetReadBuffer();
    DOUBLEBUFFER_AUTO_RELEASE_READBUFFER(&dbEnemies, pVecEnemies);


    // view angle & its components
    qangle qViewAngles; I::iEngine->GetViewAngles(qViewAngles);
    vec vForward, vRight, vUp; Maths::AngleVectors(qViewAngles, &vForward, &vRight, &vUp);


    // Count of how many ESPs, Hitboxes & skeletons we have drawn.
    size_t nEspBoxes = 0LLU, nHitboxes = 0LLU, nSkeletons = 0LLU;


    size_t nEntities = pVecEnemies->size();
    size_t iEntIndex = 0LLU;
    for (iEntIndex = 0LLU; iEntIndex < nEntities; iEntIndex++)
    {
        BaseEntity* pEnt = (*pVecEnemies)[iEntIndex];


        // do we have atleast 1 record for this entity
        std::deque<BackTrackRecord_t>* pQRecords = F::entityIterator.GetBackTrackRecord(pEnt);
        if (pQRecords->size() == 0LLU)
            continue;
        BackTrackRecord_t& record = pQRecords->front();


        // Draw this player's esp.
        if(bDrawEsp == true)
        {
            _DrawPlayerEsp(pEnt, iEntIndex, vRight, record);
            nEspBoxes++;
        }


        // Drawing player's hitboxes.
        if (bDrawHitbox == true)
        {
            _DrawPlayerHitbox(pEnt, iEntIndex, record);
            nHitboxes++;
        }
    }


    // Now we need to also disable unused draw objects, so we don't see ghosts esp's.
    size_t nTotalEspBoxes  = m_vecPlayerEsp.size();
    size_t nTotalHitBoxes  = m_vecPlayerHitbox.size();
    size_t nTotalSkeletons = 0LLU; // comming soon.

    size_t iMaxIndex = Maths::MAX<size_t>(Maths::MAX<size_t>(nTotalEspBoxes, nTotalHitBoxes), nTotalSkeletons);
    size_t iMinIndex = Maths::MIN<size_t>(Maths::MIN<size_t>(nEspBoxes, nHitboxes),           nSkeletons);
    for (size_t i = iMinIndex; i < iMaxIndex; i++)
    {
        // disable esp
        if (i >= nEspBoxes && i < nTotalEspBoxes)
        {
            m_vecPlayerEsp[i]->SetVisible(false);
        }

        // disable hitbox
        if (i >= nHitboxes && i < nTotalHitBoxes)
        {
            for (int iDrawObjIndex = 0; iDrawObjIndex < m_vecPlayerHitbox[i].m_nHitbox; iDrawObjIndex++)
            {
                m_vecPlayerHitbox[i].m_hitboxDrawObj[iDrawObjIndex]->SetVisible(false);
            }
        }
        
        // disable skeleton
        // TODO : do this one too.
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::_DrawEspList(std::vector<IDrawObj_t*>& pVecEsp, std::vector<BaseEntity*>* pVecEntities, float flRGBSpeed, float flWidthOffset, float flHeightOffset)
{
    // Engine angles.
    qangle qViewAngles; I::iEngine->GetViewAngles(qViewAngles); qViewAngles.roll = 0.0f;
    vec vForward, vRight, vUp; Maths::AngleVectors(qViewAngles, &vForward, &vRight, &vUp);
    vRight.NormalizeInPlace();

    
    // Iterate all entities & set vertex for each.
    size_t nEntities = pVecEntities->size();
    size_t iEntIndex = 0;
    for (; iEntIndex < nEntities; iEntIndex++)
    {
        // In case, there is no box for this entity, add one.
        if (iEntIndex >= pVecEsp.size())
        {
            pVecEsp.push_back(new Box3D_t());
        }


        // Calculating min & max for the box...
        BaseEntity*     pEnt        = (*pVecEntities)[iEntIndex];
        ICollideable_t* pCollidable = pEnt->GetCollideable();
        vec             vOrigin     = pCollidable->GetCollisionOrigin();
        vec             vMin        = pCollidable->OBBMins();
        vec             vMax        = pCollidable->OBBMaxs();


        float flEntWidth  = vMax.Dist2Dto(vMin) + flWidthOffset;
        vec   vEntRight   = vRight * (flEntWidth / 2.0f);
        vec   vEntCenter  = vOrigin + ((vMin + vMax) / 2.0f);
        float flEntHeight = fabsf(vEntCenter.z - (vOrigin.z + vMin.z));
        vec   vBoxMin(vEntCenter + (vEntRight * -1.0f)); vBoxMin.z = vEntCenter.z + flEntHeight + (flHeightOffset / 2.0f);
        vec   vBoxMax(vEntCenter + vEntRight);           vBoxMax.z = vEntCenter.z - flEntHeight - (flHeightOffset / 2.0f);


        // Setting box'x min and max
        Box3D_t* pBox = reinterpret_cast<Box3D_t*>(pVecEsp[iEntIndex]); 
        pBox->SetVertex(vBoxMin, vBoxMax, qViewAngles); 
        pBox->SetColor(255, 255, 255, 255);
        pBox->SetVisible(true);
        pBox->SetRGBAnimSpeed(flRGBSpeed);
    }


    // Now we iterate the rest of the boxes and set the not visible.
    _DisableAllEsp(pVecEsp, iEntIndex);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::_DisableAllEsp(std::vector<IDrawObj_t*>& pVecEsp, size_t iStartIndex = 0LLU)
{
    size_t nElements = pVecEsp.size();
    for (size_t iIndex = iStartIndex; iIndex < nElements; iIndex++)
    {
        pVecEsp[iIndex]->SetVisible(false);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::_DrawPlayerEsp(BaseEntity* pEnt, size_t iEspIndex, const vec& vViewAngleRight, const BackTrackRecord_t& record)
{
    if (iEspIndex >= m_vecPlayerEsp.size())
    {
        m_vecPlayerEsp.push_back(new Box3D_t());
        
        // Still not enough draw objects?
        if (iEspIndex >= m_vecPlayerEsp.size())
            return;
    }


    // Model
    const model_t* pModel = pEnt->GetModel();
    if (pModel == nullptr)
        return;

    // Studio model for this entities model
    const StudioHdr_t* pStudioModel = I::iVModelInfo->GetStudiomodel(pModel);
    if (pStudioModel == nullptr)
        return;

    // Hit boxes for this studio model.
    const auto* pHitBoxSet = pStudioModel->pHitboxSet(pEnt->m_nHitboxSet());
    if (pHitBoxSet == nullptr)
        return;


    const matrix3x4_t& anchorBone     = record.m_bones[pHitBoxSet->pHitbox(HitboxPlayer_t::HitboxPlayer_Hip)->bone];
    const matrix3x4_t& tallestBone    = record.m_bones[pHitBoxSet->pHitbox(HitboxPlayer_t::HitboxPlayer_Head)->bone];
    float              flEntHeight    = pEnt->m_vecViewOffset().z;
    float              flHeightOffset = Features::Materials::Player::ESPPlayer_HeightOffset.GetData().m_flVal;


    // ESP's min & max.
    constexpr float PLAYER_ESP_WIDTH_IN_HU = 20.0f;
    float           flPlayerEspWidth = PLAYER_ESP_WIDTH_IN_HU + (Features::Materials::Player::ESPPlayer_WidthOffset.GetData().m_flVal / 2.0f);
    vec vEntCenter = tallestBone.GetWorldPos(); vEntCenter.z -= (flEntHeight / 2.0f);// +10.0f; // +10 cause the head bone pos is not the top of head, and we need to compensate for that shit.
    vec vMin(anchorBone.GetWorldPos() + (vViewAngleRight * flPlayerEspWidth));  vMin.z = vEntCenter.z + (flEntHeight + flHeightOffset) / 2.0f;
    vec vMax(anchorBone.GetWorldPos() + (vViewAngleRight * -flPlayerEspWidth)); vMax.z = vEntCenter.z - (flEntHeight + flHeightOffset) / 2.0f;


    // Drawing esp box & managing user prefrences.
    Box3D_t* pBox = reinterpret_cast<Box3D_t*>(m_vecPlayerEsp[iEspIndex]);
    pBox->SetVertex(vMin, vMax, qangle(0.0f));
    pBox->SetColor(Features::Materials::Player::ESPPlayer_ESPColorTopLeft.GetData().GetAsBytes(),     Box3D_t::VertexType_TopLeft);
    pBox->SetColor(Features::Materials::Player::ESPPlayer_ESPColorTopRight.GetData().GetAsBytes(),    Box3D_t::VertexType_TopRight);
    pBox->SetColor(Features::Materials::Player::ESPPlayer_ESPColorBottomLeft.GetData().GetAsBytes(),  Box3D_t::VertexType_BottomLeft);
    pBox->SetColor(Features::Materials::Player::ESPPlayer_ESPColorBottomRight.GetData().GetAsBytes(), Box3D_t::VertexType_BottomRight);
    pBox->SetRGBAnimSpeed(Features::Materials::Player::ESPPlayer_RGB.GetData().m_flVal);
    pBox->SetVisible(true);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void ESP_t::_DrawPlayerHitbox(BaseEntity* pEnt, size_t iEspIndex, const BackTrackRecord_t& record)
{
    if (iEspIndex >= m_vecPlayerHitbox.size())
    {
        m_vecPlayerHitbox.emplace_back();

        // still not enough?
        if (iEspIndex >= m_vecPlayerHitbox.size())
            return;
    }


    // Model
    const model_t* pModel = pEnt->GetModel();
    if (pModel == nullptr)
        return;

    // Studio model for this entities model
    const StudioHdr_t* pStudioModel = I::iVModelInfo->GetStudiomodel(pModel);
    if (pStudioModel == nullptr)
        return;

    // Hit boxes for this studio model.
    const auto* pHitBoxSet = pStudioModel->pHitboxSet(pEnt->m_nHitboxSet());
    if (pHitBoxSet == nullptr)
        return;


    HitboxDrawObj_t& hitboxDrawObjSet = m_vecPlayerHitbox[iEspIndex];
    for (int i = 0; i < hitboxDrawObjSet.m_nHitbox; i++)
    {
        Cube3D_t*      pCube       = reinterpret_cast<Cube3D_t*>(hitboxDrawObjSet.m_hitboxDrawObj[i]);
        mstudiobbox_t* pHitbox     = pHitBoxSet->pHitbox(i);
        vec            vBoneOrigin = record.m_bones[pHitbox->bone].GetWorldPos();
        pCube->SetVertex(pHitbox->bbmin, pHitbox->bbmax, record.m_bones[pHitbox->bone]);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorTopLeft.GetData().GetAsBytes(),     Cube3D_t::VertexType_TopLeftBack);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorTopLeft.GetData().GetAsBytes(),     Cube3D_t::VertexType_TopLeftFront);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorTopRight.GetData().GetAsBytes(),    Cube3D_t::VertexType_TopRightBack);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorTopRight.GetData().GetAsBytes(),    Cube3D_t::VertexType_TopRightFront);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorBottomLeft.GetData().GetAsBytes(),  Cube3D_t::VertexType_BottomLeftBack);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorBottomLeft.GetData().GetAsBytes(),  Cube3D_t::VertexType_BottomLeftFront);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorBottomRight.GetData().GetAsBytes(), Cube3D_t::VertexType_BottomRightBack);
        pCube->SetColor(Features::Materials::Player::ESPPlayer_ExtraColorBottomRight.GetData().GetAsBytes(), Cube3D_t::VertexType_BottomRightFront);
        pCube->SetRGBAnimSpeed(Features::Materials::Player::ESPPlayer_SkeletonRGB.GetData().m_flVal);
        pCube->SetVisible(true);
    }
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
HitboxDrawObj_t::HitboxDrawObj_t()
{
    m_nHitbox = HitboxPlayer_Count;
    InitDrawObjs();
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void HitboxDrawObj_t::InitDrawObjs()
{
    for (size_t i = 0; i < m_nHitbox; i++)
    {
        m_hitboxDrawObj[i] = new Cube3D_t();
        m_hitboxDrawObj[i]->SetVisible(false);
    }
}