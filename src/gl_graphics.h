#ifndef GL_GRAPHICS_H
#define GL_GRAPHICS_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

struct GLTilemapTexture
{
    u32 texture_id;
    i32 width, height;
    i32 rows, cols;
};

struct GLShader
{
    u32 id = 0;
    
    void apply();
    void create(const char* const vsrc, const char* const fsrc);
};

internal GLTilemapTexture
gl_load_rgba_tilemap(
                     u8* data,
                     i32 width,
                     i32 height,
                     i32 tilemap_rows,
                     i32 tilemap_cols);

internal void
gl_slow_tilemap_draw(
                     GLTilemapTexture const* tm,
                     glm::vec2 pos,
                     glm::vec2 size,
                     float rotate = 0.f,
                     i32 tm_index = 0,
                     b32 mirrorx = false,
                     b32 mirrory = false,
                     NRGBA tint = NRGBA{1.f, 1.f, 1.f, 1.f},
                     b32 account_for_offset = true);

const char* const g_2d_vs =
R"EOS(
#version 330
layout (location = 0) in vec4 vertex;

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

)EOS";
const char* const g_2d_fs =
R"EOS(
#version 330
in vec2 TexCoords;
out vec4 color;

uniform sampler2DArray sampler;
uniform int layer = 0;
uniform vec4 tint = vec4(1, 1, 1, 1);

void main()
{
      color =  texture(sampler, vec3(TexCoords,layer)) * tint;
      //color = vec4(0.0, 1.0, 0.0, 1.0);
    }
    
)EOS";

global u32 g_quad_vao;
global GLShader g_shd_2d;
global glm::mat4 g_projection;

#endif //! GL_GRAPHICS_H
