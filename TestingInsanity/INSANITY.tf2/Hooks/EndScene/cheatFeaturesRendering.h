#include "EndScene.h"
#define FOV_MULTIPLIER 10.0f

namespace directX {
    namespace render_cheat_features
    {
        inline ImColor WHITE(1.0f, 1.0f, 1.0f, 1.0f); // white color FOV circle when no valid target
        inline ImColor GREEN(0.0f, 1.0f, 0.0f, 1.0f); // green FOV circle with valid target
    }
}