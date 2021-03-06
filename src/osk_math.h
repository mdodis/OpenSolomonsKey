#ifndef OSK_MATH_H
#define OSK_MATH_H

#include <math.h>
#include <float.h>

#define D2R 0.0174533f
#define R2D 57.2958f
union ivec2
{
    struct
    {
        i32 x, y;
    };
    i32 e[2];
    
    bool operator==(const ivec2& other)
    {
        return (this->x == other.x && this->y == other.y);
    }
    
    bool operator!=(const ivec2& other)
    {
        return !(this->x == other.x && this->y == other.y);
    }
    
    ivec2 operator+(const ivec2& other)
    {
        return {x + other.x, y + other.y};
    }
    
    ivec2 operator-(const ivec2& other)
    {
        return {x - other.x, y - other.y};
    }
    
    ivec2 operator+=(const ivec2& other)
    {
        *this = *this + other;
        return *this;
    }
    
    ivec2 operator-=(const ivec2& other)
    {
        *this = *this - other;
        return *this;
    }
    
    ivec2 operator+(i32 other)
    {
        return {x + other, y + other};
    }
    
    ivec2 operator-(i32 other)
    {
        return {x - other, y - other};
    }
    
    ivec2 operator*(i32 other)
    {
        return {x * other, y * other};
    }
    
};

union fvec2
{
    struct
    {
        float x, y;
    };
    struct
    {
        float w, h;
    };
    float e[2];
    
    bool operator==(const fvec2& other)
    {
        return (this->x == other.x && this->y == other.y);
    }
    
    bool operator!=(const fvec2& other)
    {
        return (this->x != other.x || this->y != other.y);
    }
    
    
    fvec2 operator+(const fvec2& other)
    {
        return {x + other.x, y + other.y};
    }
    
    fvec2 operator-(const fvec2& other)
    {
        return {x - other.x, y - other.y};
    }
    
    fvec2 operator+=(const fvec2& other)
    {
        *this = *this + other;
        return *this;
    }
    
    fvec2 operator-=(const fvec2& other)
    {
        *this = *this - other;
        return *this;
    }
    
    fvec2 operator+(i32 other)
    {
        return {x + other, y + other};
    }
    
    fvec2 operator-(i32 other)
    {
        return {x - other, y - other};
    }
    
    fvec2 operator*(i32 other)
    {
        return {x * other, y * other};
    }
    
    fvec2 operator*(float other)
    {
        float nx = (float)( (float)x * other);
        float ny = (float)( (float)y * other);
        return {nx, ny};
    }
    
    fvec2 operator-() {
        return fvec2{-x, -y};
    }
};


fvec2 operator*(float other, fvec2 v)
{
    float nx = (float)( (float)v.x * other);
    float ny = (float)( (float)v.y * other);
    return {nx, ny};
}


/*
Box in sprite space (i.e 64 over 64)
*/
struct AABox
{
    float min_x = 0;
    float min_y = 0;
    float max_x = 64;
    float max_y = 64;
    
    AABox translate(fvec2 position) const;
};

internal b32
aabb_minkowski(
               const AABox* const a,
               const AABox* const b,
               fvec2* const opt_pen);


inline u64 lengthsq(const fvec2* const v);
inline float ftrunc(float n);
inline float iabs(float n);
//float iclamp(float a, float b, float x);
// Precise method, which guarantees v = v1 when t = 1.
float lerp(float v0, float v1, float t) {
    return (1 - t) * v0 + t * v1;
}


fvec2 lerp2(fvec2 a, fvec2 b, float t) {
    fvec2 result = {lerp(a.x, b.x, t), lerp(a.y,b.y,t)};
    return result;
}

inline int highest_pow2(int n);

#endif //!OSK_MATH_H


inline u64 lengthsq(const fvec2& v)
{
    return v.x * v.x + v.y * v.y;
}

inline float ftrunc(float n) { return (float)(n); }

inline float iabs(float n)
{
    if (n < 0)
        return -n;
    else
        return n;
}

inline float clamp(float a, float b, float x)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline fvec2 clamp(fvec2 a, fvec2 b, fvec2 x)
{
    return fvec2
    {
        clamp(a.x, b.x, x.x),
        clamp(a.y, b.y, x.y)
    };
}

inline i32 clamp(i32  a, i32 b, i32 x)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline ivec2 clamp(ivec2 a, ivec2 b, ivec2 x)
{
    return ivec2
    {
        clamp(a.x, b.x, x.x),
        clamp(a.y, b.y, x.y)
    };
}

inline float mirror_x_axis(float x) {
    float ratio = ((x/960.f)*2.f) - 1.f;
    
    float result = ((-ratio + 1.f)/2.f) * 960.f;
    return result;
}

inline int highest_pow2(int n)
{
    int p = (int)log2(n);
    return (int)pow(2, p);
}

inline ivec2
map_position_to_tile_centered(fvec2 position)
{
    return ivec2{((i32)position.x + 32) / 64, ((i32)position.y + 32) / 64};
}

inline fvec2 direction_from_rotation( float theta){
    fvec2 result = fvec2{ roundf(cosf(theta)),  roundf(sinf(theta))  };
    
    return result;
}

inline ivec2 get_tile_behind(ivec2 t, fvec2 tv) {
    tv = fvec2{-tv.x, -tv.y};
    
    return t + ivec2{(int)tv.x, (int)tv.y};
}

inline fvec2
tile_to_position(ivec2 tile) {
    return fvec2{tile.x * 64.f, tile.y * 64.f};
}

inline ivec2
map_position_to_tile(fvec2 position) {
    return ivec2{((i32)position.x) / 64, ((i32)position.y) / 64};
}

AABox AABox::translate(fvec2 position) const {
    return AABox {
        min_x + position.x,
        min_y + position.y,
        min_x + position.x + max_x,
        min_y + position.y + max_y
    };
}

internal AABox
get_tile_box(ivec2 t) {
    return AABox{
        t.x * 64.f, t.y * 64.f,
        t.x * 64.f + 64.f, t.y * 64.f + 64.f
    };
}

internal b32
intersect(const AABox* const a, const AABox* const b) {
    AABox result;
    result.min_y = a->min_y - b->max_y;
    result.max_y = a->max_y - b->min_y;
    result.min_x = a->min_x - b->max_x;
    result.max_x = a->max_x - b->min_x;
    
    return (result.min_x <= 0 &&
            result.max_x >= 0 &&
            result.min_y <= 0 &&
            result.max_y >= 0
            );
}

internal b32
aabb_minkowski(const AABox* const a,
               const AABox* const b,
               fvec2* const opt_pen) {
    AABox result;
    result.min_y = a->min_y - b->max_y;
    result.max_y = a->max_y - b->min_y;
    result.min_x = a->min_x - b->max_x;
    result.max_x = a->max_x - b->min_x;
    
    
    if (result.min_x <= 0 &&
        result.max_x >= 0 &&
        result.min_y <= 0 &&
        result.max_y >= 0) {
        
        fvec2 penetration = {0,0};
        float min = FLT_MAX;
        /*NOTE(mdodis): prefer Y axis over X on collision*/
        
        if (abs(result.min_x) < min) {
            min = abs(result.min_x);
            penetration = {result.min_x, 0};
        }
        
        if (abs(result.max_x) < min) {
            min = abs(result.max_x);
            penetration = {result.max_x, 0};
        }
        
        if (abs(result.min_y) < min) {
            min = abs(result.min_y);
            penetration = {0, result.min_y};
        }
        
        if (abs(result.max_y) < min) {
            min = abs(result.max_y);
            penetration = {0, result.max_y};
        }
        
        if (opt_pen) *opt_pen = penetration;
        
        return true;
    }
    
    if (opt_pen) *opt_pen = {0,0};
    
    return false;
}



inline float sgn(float a) {
    if (a < 0) return -1;
    else return 1;
}


inline i32 sgn(i32 a) {
    if (a < 0) return -1;
    else return 1;
}

inline float min(float a, float b) {
    return a < b ? a : b;
}

inline float dot(fvec2 a, fvec2 b) {
    return a.x * b.x + a.y * b.y;
}

inline float length(fvec2 a) {
    return sqrt(a.x * a.x + a.y * a.y);
}

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

inline float osk__min(float a, float b, float on_zero) {
    if (a < 0.f)
        return (b > 0) ? (b) : on_zero;
    if (b < 0.f)
        return (a > 0) ? (a) : on_zero;
    
    return MIN(a, b);
}

inline float deg_0_360(float a) {
    if (a < 0) return a + 360.f;
    
    return (float)((int)a % 360);
}

inline fvec2 normalize(fvec2 a) {
    float l = length(a);
    return (l == 0.f ? fvec2{0,0} : fvec2{a.x / l, a.y / l});
}

inline float distance(fvec2 a, fvec2 b) {
#define SQ(x) ((x) * (x))
    return sqrtf( SQ(a.x - b.x) + SQ(a.y - b.y));
#undef SQ
}

inline float random11() {
    return ((float)rand()/RAND_MAX) * 2.f - 1.f;
}

inline float random01() {
    return ((float)rand()/RAND_MAX);
}

struct NRGBA {
    float r, g, b, a;
};

struct mat4 {
    union {
        float m[4][4];
        struct {
            float a00, a01, a02, a03,
            a10, a11, a12, a13,
            a20, a21, a22, a23,
            a30, a31, a32, a33;
        };
    };
};

NRGBA rand_nrgb() {
    NRGBA result = {
        (float)rand() / (float)RAND_MAX,
        (float)rand() / (float)RAND_MAX,
        (float)rand() / (float)RAND_MAX,
        1.f,
    };
    
    return result;
}