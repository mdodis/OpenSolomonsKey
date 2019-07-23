
void GLShader::create(const char* const vsrc, const char* const fsrc)
{
    u32 vertex, fragment;
    
    int success;
    char infoLog[512];
    
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vsrc, NULL);
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
    glShaderSource(fragment, 1, &fsrc, NULL);
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
    this->id= glCreateProgram();
    glAttachShader(this->id, vertex);
    glAttachShader(this->id, fragment);
    glLinkProgram(this->id);
    // print linking errors if any
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(this->id, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",infoLog);
        assert(0);
    }
    glUseProgram(this->id);
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
}

void GLShader::apply()
{
    glUseProgram(this->id);
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

internal GLTilemapTexture
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
    
    return GLTilemapTexture{tilemap_id, width, height, tilemap_rows, tilemap_cols};
}


internal void
gl_slow_tilemap_draw(
GLTilemapTexture const* tm,
glm::vec2 pos,
glm::vec2 size,
float rotate,
i32 tm_index,
b32 mirrorx,
b32 mirrory)
{
    
    glBindTexture(GL_TEXTURE_2D_ARRAY, tm->texture_id);
    glBindVertexArray(g_quad_vao);
    glUseProgram(g_shd_2d.id);
    
    GLuint layer_loc = glGetUniformLocation(g_shd_2d.id, "layer");
    glUniform1i(layer_loc,tm_index );
    
    glm::mat4 model(1.0f);
    
    model = glm::scale(model, glm::vec3(g_pixel_scale, g_pixel_scale, 1.f));
    // ORIGIN on BOTTOM-CENTER
    // model = glm::translate(model, glm::vec3(pos - glm::vec2(-0.5f * size.x, size.y), 0.0f));
    // ORIGIN on TOP-LEFT
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    
    model = glm::translate(model, glm::vec3(32.f, 64.f, 0.f));
    
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
    
    GLuint loc = glGetUniformLocation(g_shd_2d.id, "model");
    glUniformMatrix4fv(
        loc,
        1,
        GL_FALSE,
        glm::value_ptr(model));
    
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
}
