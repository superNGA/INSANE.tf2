#include "AnimationHandler.h"
#include <algorithm>


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AnimationHandler_t::CalculateAnim()
{
    // If nearly complete, let it go.
    if(fabsf(ANIM_COMPLETE - m_flAnimation) < ANIM_COMPLETION_TOLERANCE)
    {
        m_flAnimation = ANIM_COMPLETE;
        return;
    }

    // Current time
    std::chrono::high_resolution_clock::time_point timeNow = std::chrono::high_resolution_clock::now();

    double flTimeSinceReset = static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(timeNow - m_startTime).count()) / 1000.0f;
    float  flTimeNormalized = static_cast<float>(flTimeSinceReset) / m_flAnimationCompleteTime;

    m_flAnimation = 1.0f - powf(1.0f - flTimeNormalized, 3.0f); // animation = 1 - (1 - time)^3
    m_flAnimation = std::clamp<float>(m_flAnimation, ANIM_ZERO, ANIM_COMPLETE);
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
void AnimationHandler_t::Reset()
{
    m_startTime   = std::chrono::high_resolution_clock::now();
    m_flAnimation = 0.0f;
}


///////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////
float AnimationHandler_t::GetAnimation() const
{
    return m_flAnimation;
}
