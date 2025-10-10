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
    enum AnimType_t : int
    {
        AnimType_Linear = 0,
        AnimType_Quadratic,
        AnimType_Cubic
    };

    AnimationHandler_t(float flAnimationCompTime = ANIM_DURATION_IN_SEC, AnimType_t iAnimationType = AnimType_Cubic)
    {
        m_startTime               = std::chrono::high_resolution_clock::now();
        m_flAnimation             = 0.0f;
        m_flAnimationCompleteTime = flAnimationCompTime;
        m_iAnimationType          = iAnimationType;
    }


    void  CalculateAnim();
    void  Reset();
    float GetAnimation() const;
    bool  IsComplete() const;

private:
    void _CalculateLinearAnim();
    void _CalculateCubicAnim();

    std::chrono::high_resolution_clock::time_point m_startTime;
    float                                          m_flAnimation             = 0.0f;
    float                                          m_flAnimationCompleteTime = ANIM_DURATION_IN_SEC;
    AnimType_t                                     m_iAnimationType          = AnimType_Cubic;
};
///////////////////////////////////////////////////////////////////////////
