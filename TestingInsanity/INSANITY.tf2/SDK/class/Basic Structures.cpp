#include "Basic Structures.h"

#include <cmath>
#include <algorithm>
#include "../../Extra/math.h"
#include "../../External Libraries/ImGui/imgui.h"

////////////////////////////// QAngle ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
void qangle::Init()
{
    pitch = 0.0f; yaw = 0.0f; roll = 0.0f;
}


qangle qangle::operator+(qangle other)
{
    return qangle(pitch + other.pitch, yaw + other.yaw, 0.0f);
}


qangle qangle::operator-(qangle other)
{
    return qangle(pitch - other.pitch, yaw - other.yaw, 0.0f);
}


qangle& qangle::operator=(qangle other)
{
    pitch = other.pitch; yaw = other.yaw; roll = other.roll;
    return *this;
}


////////////////////////////// RGBA_t ////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
RGBA_t::RGBA_t(float R, float G, float B, float A)
{
    r = static_cast<unsigned char>(R * 255.0f);
    g = static_cast<unsigned char>(G * 255.0f);
    b = static_cast<unsigned char>(B * 255.0f);
    a = static_cast<unsigned char>(A * 255.0f);
}


RGBA_t::RGBA_t(const ImVec4& vClr)
{
    r = static_cast<unsigned char>(vClr.x * 255.0f);
    g = static_cast<unsigned char>(vClr.y * 255.0f);
    b = static_cast<unsigned char>(vClr.z * 255.0f);
    a = static_cast<unsigned char>(vClr.w * 255.0f);
}


void RGBA_t::Init()
{
    r = 0; g = 0; b = 0; a = 0xFF;
}


RGBA_t& RGBA_t::operator=(RGBA_t other)
{
    r = other.r; g = other.g; b = other.b; a = other.a;
    return *this;
}


bool RGBA_t::operator==(const RGBA_t other) const
{
    return (r == other.r && g == other.g && b == other.b && a == other.a);
}


RGBA_t RGBA_t::IncreaseInPlace(int iOffset, bool bColors, bool bAlpha)
{
    if (bColors == true)
    {
        r = static_cast<unsigned char>(std::clamp<int>(static_cast<int>(r) + iOffset, 0, 0xFF));
        g = static_cast<unsigned char>(std::clamp<int>(static_cast<int>(g) + iOffset, 0, 0xFF));
        b = static_cast<unsigned char>(std::clamp<int>(static_cast<int>(b) + iOffset, 0, 0xFF));
    }

    if (bAlpha == true)
    {
        a = static_cast<unsigned char>(std::clamp<int>(static_cast<int>(a) + iOffset, 0, 0xFF));
    }

    return *this;
}


RGBA_t RGBA_t::IncreaseClr(int iOffset, bool bColors, bool bAlpha) const
{
    return RGBA_t(
            bColors == true ? static_cast<unsigned char>(std::clamp<int>(static_cast<int>(r) + iOffset, 0, 0xFF)) : r,
            bColors == true ? static_cast<unsigned char>(std::clamp<int>(static_cast<int>(g) + iOffset, 0, 0xFF)) : g,
            bColors == true ? static_cast<unsigned char>(std::clamp<int>(static_cast<int>(b) + iOffset, 0, 0xFF)) : b,
            bAlpha  == true ? static_cast<unsigned char>(std::clamp<int>(static_cast<int>(a) + iOffset, 0, 0xFF)) : a);
}


void RGBA_t::LerpInPlace(RGBA_t target, float flPower, bool bColors, bool bAlpha)
{
    flPower = std::clamp<float>(flPower, 0.0f, 1.0f);

    if (bColors == true)
    {
        r = static_cast<unsigned char>(
            std::clamp<int32_t>(
                static_cast<int32_t>(r) + static_cast<int32_t>(static_cast<float>(static_cast<int32_t>(target.r) - static_cast<int32_t>(r)) * flPower), 0, 0xFF)
            );
        g = static_cast<unsigned char>(
            std::clamp<int32_t>(
                static_cast<int32_t>(g) + static_cast<int32_t>(static_cast<float>(static_cast<int32_t>(target.g) - static_cast<int32_t>(g)) * flPower), 0, 0xFF)
            );
        b = static_cast<unsigned char>(
            std::clamp<int32_t>(
                static_cast<int32_t>(b) + static_cast<int32_t>(static_cast<float>(static_cast<int32_t>(target.b) - static_cast<int32_t>(b)) * flPower), 0, 0xFF)
            );
    }

    if (bAlpha)
    {
        a = static_cast<unsigned char>(
            std::clamp<int32_t>(
                static_cast<int32_t>(a) + static_cast<int32_t>(static_cast<float>(static_cast<int32_t>(target.a) - static_cast<int32_t>(a)) * flPower), 0, 0xFF)
            );
    }
}


void RGBA_t::Set(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a)
{
    r = _r;  g = _g;  b = _b;  a = _a;
}


Vec4 RGBA_t::GetAsVec4() const
{
    return Vec4(
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
    );
}


ImVec4 RGBA_t::GetAsImVec4() const
{
    return ImVec4(
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
    );
}


////////////////////////////// HSVA_t /////////////////////////////////
///////////////////////////////////////////////////////////////////////
HSVA_t RGBA_t::ToHSVA() const
{
    Vec4 clr = GetAsVec4();
    float mx = Maths::MAX<float>(clr.x, Maths::MAX<float>(clr.y, clr.z ));
    float mn = Maths::MIN<float>(clr.x, Maths::MIN<float>(clr.y, clr.z));
    float d = mx - mn;

    HSVA_t out;
    out.a = clr.w;
    out.v = mx;
    out.s = (mx == 0.0f) ? 0.0f : d / mx;

    if (d == 0.0f) {
        out.h = 0.0f; // undefined hue
    }
    else if (mx == clr.x) {
        out.h = 60.0f * fmod(((clr.y - clr.z) / d), 6.0f);
    }
    else if (mx == clr.y) {
        out.h = 60.0f * (((clr.z - clr.x) / d) + 2.0f);
    }
    else {
        out.h = 60.0f * (((clr.x - clr.y) / d) + 4.0f);
    }
    if (out.h < 0.0f) out.h += 360.0f;
    return out;
}


void HSVA_t::Init()
{
    h = 0.0f; s = 0.0f; v = 0.0f; a = 0.0f;
}


RGBA_t HSVA_t::ToRGBA() const
{
    float C = v * s;
    float X = C * (1.0f - fabs(fmod(h / 60.0f, 2.0f) - 1.0f));
    float m = v - C;

    float r, g, b;
    if (h < 60.0f) { r = C; g = X; b = 0; }
    else if (h < 120.0f) { r = X; g = C; b = 0; }
    else if (h < 180.0f) { r = 0; g = C; b = X; }
    else if (h < 240.0f) { r = 0; g = X; b = C; }
    else if (h < 300.0f) { r = X; g = 0; b = C; }
    else { r = C; g = 0; b = X; }

    return RGBA_t(r + m, g + m, b + m, a);
}


////////////////////////////// vec ////////////////////////////////////
///////////////////////////////////////////////////////////////////////
void vec::Init()
{
    x = 0.0f; y = 0.0f; z = 0.0f;
}


vec vec::operator+(const vec other) const
{
    return vec(x + other.x, y + other.y, z + other.z);
}


vec vec::operator+(float other) const
{
    return vec(x + other, y + other, z + other);
}


vec vec::operator-(const vec& other) const
{
    return vec(x - other.x, y - other.y, z - other.z);
}


vec vec::operator/(float other)
{
    return vec(x / other, y / other, z / other);
}


vec& vec::operator+=(vec other)
{
    x += other.x; y += other.y; z += other.z;
    return *this;
}


vec& vec::operator-=(vec other)
{
    x -= other.x; y -= other.y; z -= other.z;
    return *this;
}


vec& vec::operator*=(float other)
{
    x *= other; y *= other; z *= other;
    return *this;
}


void vec::operator=(vec other)
{
    x = other.x; y = other.y; z = other.z;
}


bool vec::operator==(const vec& other) const
{
    return (x == other.x && y == other.y && z == other.z);
}


float vec::Length()
{
    return sqrtf(x * x + y * y + z * z);
}


float vec::LengthSqrt() const
{
    return x * x + y * y + z * z;
}


float vec::Length2D() const
{
    return sqrtf(x * x + y * y);
}


float vec::DistTo(const vec& other) const
{
    return (*this - other).Length();
}


float vec::Dist2Dto(const vec& other) const
{
    return sqrtf((x - other.x) * (x - other.x) + (y - other.y) * (y - other.y));
}


bool vec::IsEmpty() const
{
    return fabs(x) < 0.0001f && fabs(y) < 0.0001f && fabs(z) < 0.0001f;
}


bool vec::IsZero() const
{
    return fabsf(x) == 0.0f && fabsf(y) == 0.0f && fabsf(z) == 0.0f;
}


vec vec::Normalize() const
{
    float flLength = sqrtf(x * x + y * y + z * z);
    return vec(x / flLength, y / flLength, z / flLength);
}


float vec::Dot(const vec& other) const
{
    return (x * other.x + y * other.y + z * other.z);
}


vec vec::CrossProduct(vec other) const
{
    return vec(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}


vec& vec::NormalizeInPlace()
{
    float flMagnitude = sqrtf(x * x + y * y + z * z);
    x /= flMagnitude; y /= flMagnitude; z /= flMagnitude;
    return *this;
}


bool vec::HasSameDirection(const vec& other) const
{
    vec vThis = this->Normalize();
    vec vOther = other.Normalize();

    return
        fabsf(vThis.x) == fabsf(vOther.x) &&
        fabsf(vThis.y) == fabsf(vOther.y) &&
        fabsf(vThis.z) == fabsf(vOther.z);
}


////////////////////////////// vecAligned /////////////////////////////////
///////////////////////////////////////////////////////////////////////////
vecAligned& vecAligned::operator= (const vec& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
}


vecAligned& vecAligned::operator= (const vecAligned& other)
{
    x = other.x;
    y = other.y;
    z = other.z;
    w = other.w;
    return *this;
}


void vecAligned::operator*=(const float other)
{
    x *= other; y *= other; z *= other; w *= other;
}


void vecAligned::operator-=(const float other)
{
    x -= other; y -= other; z -= other; w -= other;
}


void vecAligned::operator+=(const float other)
{
    x += other; y += other; z += other; w += other;
}


vecAligned vecAligned::operator+ (const vecAligned& other)
{
    return vecAligned(x + other.x, y + other.y, z + other.z);
}


vecAligned vecAligned::operator+ (const vec& other)
{
    return vecAligned(x + other.x, y + other.y, z + other.z);
}


vecAligned vecAligned::operator+ (const float other)
{
    return vecAligned(x + other, y + other, z + other);
}


vecAligned vecAligned::operator- (const vecAligned& other)
{
    return vecAligned(x - other.x, y - other.y, z - other.z);
}


vecAligned vecAligned::operator- (const vec& other)
{
    return vecAligned(x - other.x, y - other.y, z - other.z);
}


vecAligned vecAligned::operator- (const float other)
{
    return vecAligned(x - other, y - other, z - other);
}


////////////////////////////// Vec2 ////////////////////////////////////
////////////////////////////////////////////////////////////////////////
const Vec2& Vec2::operator=(Vec2 other)
{
    x = other.x; y = other.y;
    return *this;
}


bool Vec2::operator==(Vec2 other) const
{
    return x == other.x && y == other.y;
}


bool Vec2::IsEmpty() const
{
    return fabsf(x) < 0.0001f && fabsf(y) < 0.0001f;
}


Vec2 Vec2::operator+(Vec2 other) const
{
    return Vec2(x + other.x, y + other.y);
}


Vec2& Vec2::operator+=(Vec2 other)
{
    x += other.x; y += other.y;
    return *this;
}


Vec2 Vec2::operator-(Vec2 other) const
{
    return Vec2(x - other.x, y - other.y);
}


Vec2& Vec2::operator-=(Vec2 other)
{
    x -= other.x; y -= other.y;
    return *this;
}


////////////////////////////// Vec4 ////////////////////////////////////
////////////////////////////////////////////////////////////////////////
Vec4::Vec4(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    x = static_cast<float>(r) / 255.0f;
    y = static_cast<float>(g) / 255.0f;
    z = static_cast<float>(b) / 255.0f;
    w = static_cast<float>(a) / 255.0f;
}


void Vec4::Init()
{
    x = 0.0f; y = 0.0; z = 0.0f; w = 0.0f;
}


void Vec4::Set(unsigned char r, unsigned char g, unsigned char b, unsigned char a)
{
    x = static_cast<float>(r) / 255.0f;
    y = static_cast<float>(g) / 255.0f;
    z = static_cast<float>(b) / 255.0f;
    w = static_cast<float>(a) / 255.0f;
}


////////////////////////////// view_matrix ////////////////////////////////
///////////////////////////////////////////////////////////////////////////
view_matrix::view_matrix()
{
    m[0][0] = 0.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
    m[1][0] = 0.0f; m[1][1] = 0.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
    m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 0.0f; m[2][3] = 0.0f;
    m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 0.0f;
}


const view_matrix& view_matrix::operator=(const view_matrix& other)
{
    m[0][0] = other.m[0][0];   m[0][1] = other.m[0][1];   m[0][2] = other.m[0][2];   m[0][3] = other.m[0][3];
    m[1][0] = other.m[1][0];   m[1][1] = other.m[1][1];   m[1][2] = other.m[1][2];   m[1][3] = other.m[1][3];
    m[2][0] = other.m[2][0];   m[2][1] = other.m[2][1];   m[2][2] = other.m[2][2];   m[2][3] = other.m[2][3];
    m[3][0] = other.m[3][0];   m[3][1] = other.m[3][1];   m[3][2] = other.m[3][2];   m[3][3] = other.m[3][3];

    return *this;
}
