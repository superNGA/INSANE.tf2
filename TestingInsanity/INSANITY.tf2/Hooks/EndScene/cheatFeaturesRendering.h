#include "EndScene.h"
#define FOV_MULTIPLIER 10.0f

namespace directX {
    namespace render_cheat_features
    {
        inline ImColor WHITE(1.0f, 1.0f, 1.0f, 1.0f); // white color FOV circle when no valid target
        inline ImColor GREEN(0.0f, 1.0f, 0.0f, 1.0f); // green FOV circle with valid target

        inline void render_esp_boxes(ImDrawList* drawList, std::vector<entInfo_t>& CHE_vecEntInfo) {

            if (CHE_vecEntInfo.empty()) return;
            view_matrix CHE_viewMatrix = entities::M_worldToScreen.load();

            for (auto& ent : CHE_vecEntInfo) {

                // skipping disguised spy if enabled feature
                if (config.visualConfig.skipDisguisedSpy && ent.getFlagBit(IS_DISGUISED)) {
                    continue;
                }

                // skipping cloaked spy if enabled feature
                if (config.visualConfig.skipCloackedSpy && ent.getFlagBit(IS_CLOAKED)) {
                    continue;
                }

                int8_t onScreenCounter = 0; // is this is still 0 after performaing world-to-screen, then entity is not on screen. 

                /* going from first bone HEAD to LAST bone, doing world-to-screen for each bone */
                for (int INDEX_boneScreenPos = HEAD; INDEX_boneScreenPos <= PELVIS; INDEX_boneScreenPos++) {

                    vec vecCurBoneCoordinates = ent.bones[INDEX_boneScreenPos].get_bone_coordinates(); // bone's world coordintates ( without adjustment for esp )

                    // Adjusting bone screen position to make esp proper :)
                    if (INDEX_boneScreenPos == HEAD) {
                        vecCurBoneCoordinates.z += 15.0f;
                    }

                    // getting screen pos for adjusted bone positions 
                    if (entities::world_to_screen(
                        vecCurBoneCoordinates, // entity bone position
                        ent.boneScreenPos[INDEX_boneScreenPos], // where we want to store the bone screen position
                        &CHE_viewMatrix)) {
                        onScreenCounter++;
                    }
                }
                // if onScreenCounter is zero, then entity is not on screen, don't draw anything for that entity
                if (!onScreenCounter) { // ON_SCREEN bit will be 0 by default so not calling clear bit function redundantly
                    continue;
                }
                ent.setFlagBit(ON_SCREEN);

                /* drawing ESP rectangle for current entity */
                float entWidth = abs(ent.boneScreenPos[HEAD].y - ent.boneScreenPos[PELVIS].y);
                float entHeight = entWidth * 2.25f;

                // ESP box's screen coordinates
                ImVec2 topLeft(ent.boneScreenPos[HEAD].x - entWidth / 2.0f, ent.boneScreenPos[HEAD].y);
                ImVec2 bottomRight(ent.boneScreenPos[HEAD].x + entWidth / 2.0f, ent.boneScreenPos[HEAD].y + entHeight);
                ImVec2 topRight(topLeft.x + entWidth, topLeft.y);
                ImVec2 bottomLeft(bottomRight.x - entWidth, bottomRight.y);

                // ESP box
                drawList->AddRect(
                    topLeft, // TOP-LEFT corner for esp box
                    bottomRight, // BOTTOM-RIGHT corner 
                    ImColor(ent.getFlagBit(SHOULD_LOCK_AIMBOT) ? GREEN : WHITE) // color
                );

                // HEALTH BAR
                if (config.visualConfig.healthBar) {

                    float healthBarWidth = entWidth * 0.2f;

                    // empty health bar
                    drawList->AddRect(
                        ImVec2(topLeft.x - healthBarWidth, topLeft.y),
                        ImVec2(bottomLeft.x - healthBarWidth * 0.2f, bottomLeft.y),
                        WHITE
                    );

                    // filled health bar accoring to entities health
                    drawList->AddRectFilled(
                        ImVec2(topLeft.x - healthBarWidth, bottomLeft.y - entHeight * ((float)ent.health / (float)ent.maxHealth)),
                        ImVec2(bottomLeft.x - healthBarWidth * 0.2f, bottomLeft.y),
                        GREEN
                    );
                }

                // PLAYER NAME
                if (config.visualConfig.playerName) {

                    ImVec2 vec_nameSize = ImGui::CalcTextSize(ent.entUserName.c_str());
                    ImVec2 vec_namePos(
                        (topRight.x + topLeft.x) / 2 - vec_nameSize.x,
                        topLeft.y + vec_nameSize.y
                    );
                    drawList->AddText(vec_namePos, IM_COL32(255, 255, 255, 255), ent.entUserName.c_str());
                }

                // testing visibility
                if (ent.getFlagBit(IS_VISIBLE)) {
                    drawList->AddText(topRight, IM_COL32(255, 255, 255, 255), "visible");
                }
                else {
                    drawList->AddText(topRight, IM_COL32(255, 255, 255, 255), "NOPE");
                }

            }
        }

        inline void render_FOV_circle(ImDrawList* drawList) {

            //drawList->AddCircle(
            //    ImVec2(global::window_size.x / 2.0f, global::window_size.y / 2.0f), // center of screen
            //    entities::FOVToRadius(config::aimbot::FOV, 120.0f, global::window_size.y),
            //    entities::shouldDoAimbot.load() ? WHITE : GREEN, // color
            //    0, // num_segments
            //    2.0f // thickness
            //);
        }

        inline void render_proj_helper(ImDrawList* draw_list)
        {

        }
    }
}