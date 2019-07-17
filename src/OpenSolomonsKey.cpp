#include <stdio.h>
#include <GL/gl.h>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::mat4 g_projection;

u32 g_shd_2d;
u32 g_quad_vao;

TilemapTexture g_tilemap_texture;
Tilemap        g_tilemap;

inline i32 ftrunc(float n) { return (i32)(n + 0.5f); }

/* Calculate aspect ratio from current window dimensions.
 Returns the size of a tile in pixels. For example, a window
 of dimensions (1024x896) will get a tilescale of 64 pixels
*/
internal double
get_tilescale_and_dimensions(
u32 current_w, u32 current_h,
u32* out_w, u32* out_h)
{
    i32 vw, vh;
    int leftover;
    
    if (((double)current_w / (double)current_h) == W_2_H)
    {
        vw = current_w;
        vh = current_h;
        glViewport(0,0,current_w, current_h);
        
    } else {
        
        if (current_w > current_h)
        {
            vh = current_h;
            vw = (i32)((double)(vh) * W_2_H);
            vw = ftrunc(vw);
            
            leftover = (i32)current_w - (i32)vw;
            if (leftover < 0)
            {
                // if the new one is bigger than what we show
                vh += leftover;
                vw = (i32)((double)(vh) * W_2_H);
                vw = ftrunc(vw);
            }
            leftover = (i32)current_w - (i32)vw;
            assert(leftover >= 0);
            
            glViewport(leftover / 2, 0, vw, vh);
        } else {
            vw = current_w;
            vh = (i32)((double)(vw) * HEIGHT_2_WIDTH_SCALE);
            vh = ftrunc(vh);
            
            leftover = (i32)current_h - (i32)vh;
            assert(leftover >= 0);
            
            glViewport(0, leftover / 2, vw, vh);
        }
        
    }
    
    *out_w = vw;
    *out_h = vh;
    
    return (int)vw * (0.0625);
}


internal u8*
load_image_as_rgba_pixels(
const char* const name,
i32* out_width,
i32* out_height,
i32* out_n)
{
    int i_w, i_h, i_n;
    unsigned char* data = stbi_load(name, out_width, out_height, out_n, 4);
    
    return data;
}



internal u32
gl_load_rgba_texture(u8* data, i32 width, i32 height)
{
    u32 result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(
        GL_TEXTURE_2D,
        0,
        GL_RGBA,
        width, height,
        0,
        GL_RGBA,
        GL_UNSIGNED_BYTE,
        data);
    
    assert(glGetError() == GL_NO_ERROR);
    
    return result;
}


internal TilemapTexture
gl_load_rgba_tilemap(
u8* data, 
i32 width,
i32 height,
i32 tilemap_rows,
i32 tilemap_cols)
{
    u32 tilemap_id;
    
    glGenTextures(1, &tilemap_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tilemap_id);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL,  1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE); 
    
    i32 tile_width = width / tilemap_cols;
    i32 tile_height = height / tilemap_rows;
    
    glTexStorage3D(
        GL_TEXTURE_2D_ARRAY,
        1, GL_RGBA8,
        tile_width, tile_height,
        tilemap_rows * tilemap_cols);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, height);
    
    for (i32 x = 0; x < tilemap_cols; x++)
    {
        for (i32 y = 0; y < tilemap_rows; y++)
        {
            // target (GL_TEXTURE_2D_ARRAY)
            // miplevels 0 for just single image
            // 0 (const)
            // 0 (const)
            // x * tiles_x + y
            // tile_width
            // tiles_height
            // 1 (const)
            // pformat (GL_BGR)
            // iformat (GL_UNSIGNED_BYTE)
            // pixels + (x * tile_height * width + y * tile_width) * components
            // http://docs.gl/gl4/glTexSubImage3D
            glTexSubImage3D(
                GL_TEXTURE_2D_ARRAY,
                0, 0, 0,
                x * tilemap_cols + y,
                tile_width, tile_height, 1,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                data + (x * tile_height * width + y * tile_width) * 4);
        }
    }
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    
    assert(glGetError() == GL_NO_ERROR);
    
    return TilemapTexture{tilemap_id, width, height, tilemap_rows, tilemap_cols};
}

void
cb_init()
{
    
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    u32 vertex, fragment;
    
    int success;
    char infoLog[512];
    
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &g_2d_vs, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        assert(0);
    };
    
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &g_2d_fs, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success)
    {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",infoLog);
        assert(0);
    };
    
    // shader Program
    g_shd_2d = glCreateProgram();
    glAttachShader(g_shd_2d, vertex);
    glAttachShader(g_shd_2d, fragment);
    glLinkProgram(g_shd_2d);
    // print linking errors if any
    glGetProgramiv(g_shd_2d, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(g_shd_2d, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",infoLog);
        assert(0);
    }
    glUseProgram(g_shd_2d);
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    int w, h, n;
    u8* data = load_image_as_rgba_pixels("Phoebus.png", &w, &h, &n);
    
    printf("n: %d\n", n);
    
    assert(data);
    glActiveTexture(GL_TEXTURE0);
    g_tilemap_texture = gl_load_rgba_tilemap(data, w, h, 16, 16);
    
    GLuint sampler_loc = glGetUniformLocation(g_shd_2d, "sampler");
    glUniform1i(sampler_loc, 0);
    assert(glGetError() == GL_NO_ERROR);
    
    GLuint VBO;
    GLfloat vertices[] = { 
        // Pos      // Tex
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 0.0f, 
        
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f
    };
    
    glGenVertexArrays(1, &g_quad_vao);
    glGenBuffers(1, &VBO);
    
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(g_quad_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);  
    glBindVertexArray(0);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("Glerr: %d\n", err);
        exit(-10);
    }
    
    return;
}

#include <math.h>
int highestPowerof2(int n)
{
    int p = (int)log2(n);
    return (int)pow(2, p);
}

void
cb_resize()
{
    g_tile_scale = get_tilescale_and_dimensions(
        g_wind_width, g_wind_height,
        &g_view_width, &g_view_height);
    
#ifdef OSK_ROUND_TO_POW_64
    g_tile_scale = highestPowerof2((u64)g_tile_scale);
    
    g_view_width  = g_tile_scale * 16;
    g_view_height = (i32)((double)(g_view_width) * HEIGHT_2_WIDTH_SCALE);
    
    int lx = (i32)g_wind_width - g_view_width;
    int ly = (i32)g_wind_height - g_view_height;
    glViewport(lx / 2, ly / 2, g_view_width, g_view_height);
#endif
    
    g_projection = glm::ortho(0.0f, (float)g_view_width, (float)g_view_height, 0.0f);
    
    GLuint loc = glGetUniformLocation(g_shd_2d, "projection");
    glUniformMatrix4fv(
        loc,
        1,
        GL_FALSE,
        glm::value_ptr(g_projection));
    
    g_pixel_scale = (float)g_tile_scale / 64.0f;
}

internal void
gl_quick_tilemap_draw(
TilemapTexture const* tm,
glm::vec2 pos,
glm::vec2 size,
float rotate = 0.f,
i32 tm_index = 0,
b32 mirrorx = false,
b32 mirrory = false)
{
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, tm->texture_id);
    glBindVertexArray(g_quad_vao);
    glUseProgram(g_shd_2d);
    
    GLuint layer_loc = glGetUniformLocation(g_shd_2d, "layer");
    glUniform1i(layer_loc,tm_index );
    
    glm::mat4 model(1.0f);
    
    // ORIGIN on BOTTOM-CENTER
    // model = glm::translate(model, glm::vec3(pos - glm::vec2(-0.5f * size.x, size.y), 0.0f));
    // ORIGIN on TOP-LEFT
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    
    model = glm::translate(model, glm::vec3(0.5f * size.x, .5*size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f)); 
    model = glm::translate(model, glm::vec3(-0.5f * size.x,-.5*size.y, 0.0f)); 
    
    
    if (mirrory)
    {
        model = glm::translate(model, glm::vec3(0, 1.f * size.y, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, -1.f,1.f));
    }
    
    
    if (mirrorx)
    {
        model = glm::translate(model, glm::vec3(1.f* size.x, 0, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.f,1.f));
    }
    
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    GLuint loc = glGetUniformLocation(g_shd_2d, "model");
    glUniformMatrix4fv(
        loc,
        1,
        GL_FALSE,
        glm::value_ptr(model));
    
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

glm::vec2 position(0,0);
float rotate = -90.f;
b32 mx = false;
b32 my = false;

void
cb_render(InputState istate, float dt)
{
    glClearColor( 0.156, 0.156,  0.156, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    
    
    if (istate.move_right)
    {
        position.x += 0.2f * g_pixel_scale * dt;
    }
    else if (istate.move_left)
    {
        position.x -= 0.2f * g_pixel_scale * dt;
    }
    if (istate.move_up)
    {
        position.y -= 0.2f * g_pixel_scale * dt;
    }
    else if (istate.move_down)
    {
        position.y += 0.2f * g_pixel_scale * dt;
    }
    
    if (istate.spacebar_pressed)
        mx = false;
    else mx = true;
    if (istate.m_pressed)
        my = false;
    else my = true;
    
    glm::vec2 size(g_tile_scale,g_tile_scale);
    //rotate += dt * 0.1f;
    
    for(int i = 0; i < 16; ++i )
    {
        for(int j = 0; j < 14; ++j )
        {
            gl_quick_tilemap_draw(
                &g_tilemap_texture,
                {i * g_tile_scale, j * g_tile_scale},
                {g_tile_scale, g_tile_scale});
            
        }
    }
    
    gl_quick_tilemap_draw(
        &g_tilemap_texture,
        position, size,rotate, 1, mx, my);
    
    
    
}

