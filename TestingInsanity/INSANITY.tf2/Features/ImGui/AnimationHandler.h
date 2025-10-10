#pragma once
#include <chrono>

constexpr auto ANIM_COMPLETION_TOLERANCE = 0.005f;
constexpr auto ANIM_COMPLETE             = 1.0f;
constexpr auto ANIM_ZERO                 = 0.0f;
constexpr auto ANIM_DURATION_IN_SEC      = 0.3f;


///////////////////////////////////////////////////////////////////////////
class AnimationHandler_t
{
public:
    AnimationHandler_t(float flAnimationCompTime = ANIM_DURATION_IN_SEC)
    {
        m_startTime               = std::chrono::high_resolution_clock::now();
        m_flAnimation             = 0.0f;
        m_flAnimationCompleteTime = flAnimationCompTime;
    }


    void  CalculateAnim();
    void  Reset();
    float GetAnimation() const;
    bool  IsComplete() const;

private:
    std::chrono::high_resolution_clock::time_point m_startTime;
    float                                          m_flAnimation             = 0.0f;
    float                                          m_flAnimationCompleteTime = ANIM_DURATION_IN_SEC;
};
///////////////////////////////////////////////////////////////////////////
