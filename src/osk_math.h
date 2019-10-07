#ifndef OSK_MATH_H
#define OSK_MATH_H

#include <math.h>
#include <float.h>

union ivec2
{
    struct
    {
        i32 x, y;
    };
    struct
    {
        i32 w, h;
    };
    i32 e[2];
    
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
    
    ivec2 operator*(float other)
    {
        i32 nx = (i32)( (float)x * other);
        i32 ny = (i32)( (float)y * other);
        return {nx, ny};
    }
    
};

/*
Box in sprite space (i.e 64 over 64)
*/
struct AABox
{
    i32 min_x = 0;
    i32 min_y = 0;
    i32 max_x = 64;
    i32 max_y = 64;
    
    AABox translate(ivec2 position) const;
};

internal b32
aabb_minkowski(
const AABox* const a,
const AABox* const b,
ivec2* const opt_pen);


inline u64 lengthsq(const ivec2* const v);
inline i32 ftrunc(float n);
inline i32 iabs(i32 n);
inline i32 iclamp(i32 a, i32 b, i32 x);

inline ivec2 iclamp(ivec2 a, ivec2 b, ivec2 x);
inline int highest_pow2(int n);

#endif //!OSK_MATH_H

#ifdef OSK_MATH_IMPL

inline u64 lengthsq(const ivec2& v)
{
    return v.x * v.x + v.y * v.y;
}

inline i32 ftrunc(float n) { return (i32)(n); }

inline i32 iabs(i32 n) 
{
    if (n < 0)
        return -n;
    else
        return n;
}

inline i32 iclamp(i32 a, i32 b, i32 x)
{
    if (x < a) return a;
    if (x > b) return b;
    return x;
}

inline ivec2 iclamp(ivec2 a, ivec2 b, ivec2 x)
{
    return ivec2
    {
        iclamp(a.x, b.x, x.x),
        iclamp(a.y, b.y, x.y)
    };
}

inline int highest_pow2(int n)
{
    int p = (int)log2(n);
    return (int)pow(2, p);
}

inline ivec2 
map_position_to_tile_centered(ivec2 position)
{
    // NOTE(miked): behavior of original game
    return ivec2{(position.x + 32) / 64, (position.y + 32) / 64};
}


inline ivec2 
map_position_to_tile(ivec2 position)
{
    return ivec2{(position.x) / 64, (position.y) / 64};
}

AABox AABox::translate(ivec2 position) const
{
    return AABox
    {
        min_x + position.x,
        min_y + position.y,
        min_x + position.x + max_x,
        min_y + position.y + max_y
    };
}

internal b32
intersect(const AABox* const a, const AABox* const b)
{
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
aabb_minkowski(
const AABox* const a,
const AABox* const b,
ivec2* const opt_pen)
{
    AABox result;
    result.min_y = a->min_y - b->max_y;
    result.max_y = a->max_y - b->min_y;
    result.min_x = a->min_x - b->max_x;
    result.max_x = a->max_x - b->min_x;
    
    
    if (result.min_x <= 0 &&
        result.max_x >= 0 &&
        result.min_y <= 0 &&
        result.max_y >= 0)
    {
        ivec2 penetration = {0,0};
        i32 min = INT_MAX;
        /*NOTE(miked): prefer Y axis over X on collision*/
        
        if (glm::abs(result.min_x) < min)
        {
            min = glm::abs(result.min_x);
            penetration = {result.min_x, 0};
        }
        
        if (glm::abs(result.max_x) < min)
        {
            min = glm::abs(result.max_x);
            penetration = {result.max_x, 0};
        }
        
        if (glm::abs(result.min_y) < min)
        {
            min = glm::abs(result.min_y);
            penetration = {0, result.min_y};
        }
        
        if (glm::abs(result.max_y) < min)
        {
            min = glm::abs(result.max_y);
            penetration = {0, result.max_y};
        }
        
        if (opt_pen) *opt_pen = penetration;
        
        return true;
    }
    
    if (opt_pen) *opt_pen = {0,0};
    
    return false;
}


inline i32 idot(ivec2 a, ivec2 b)
{
    return a.x * b.x + a.y * b.y;
}

inline float ilength(ivec2 a)
{
    return sqrt(a.x * a.x + a.y * a.y);
}

struct NRGBA
{
    float r, g, b, a;
};


#endif