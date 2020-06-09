
void GLShader::create(const char* const vsrc, const char* const fsrc) {
    u32 vertex, fragment;
    
    int success;
    char infoLog[512];
    
    // vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vsrc, NULL);
    glCompileShader(vertex);
    // print compile errors if any
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    
    if(!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        exit(0);
    };
    
    // fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fsrc, NULL);
    glCompileShader(fragment);
    // print compile errors if any
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",infoLog);
        assert(0);
    }
    
    // shader Program
    this->id= glCreateProgram();
    glAttachShader(this->id, vertex);
    glAttachShader(this->id, fragment);
    glLinkProgram(this->id);
    // print linking errors if any
    glGetProgramiv(this->id, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(this->id, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",infoLog);
        exit(0);
    }
    glUseProgram(this->id);
    // delete the shaders as they're linked into our program now and no longer necessery
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
}

void GLShader::apply() {
    glUseProgram(this->id);
}

internal u32 gl_load_rgba_texture(u8* data, i32 width, i32 height) {
    u32 result;
    glGenTextures(1, &result);
    glBindTexture(GL_TEXTURE_2D, result);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    fail_unless(glGetError() == GL_NO_ERROR, "");
    
    return result;
}


internal void
gl_update_rgba_texture(u8* data, i32 width, i32 height, GLuint tex) {
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        error("AA err %x\n",err);
        //exit(0);
    }
    glBindTexture(GL_TEXTURE_2D, tex);
    
    err = glGetError();
    if (err != GL_NO_ERROR) {
        error("BB err %x\n",err);
        //exit(0);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    
    //assert(err == GL_NO_ERROR);
    
}


internal GLTilemapTexture gl_load_rgba_tilemap(u8* data, i32 width, i32 height, i32 tilemap_rows, i32 tilemap_cols) {
    u32 tilemap_id;
    
    glGenTextures(1, &tilemap_id);
    glBindTexture(GL_TEXTURE_2D_ARRAY, tilemap_id);
    
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAX_LEVEL,  1);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);
    
    i32 tile_width = width / tilemap_cols;
    i32 tile_height = height / tilemap_rows;
    i32 tiles = tilemap_rows * tilemap_cols;
    
    glTexStorage3D(GL_TEXTURE_2D_ARRAY,1, GL_RGBA8, tile_width, tile_height, tiles);
    
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   width);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, height);
    
    u8* c = data;
    
    i32 count = 0;
    for (i32 y = 0; y < tilemap_rows; y++) {
        for (i32 x = 0; x < tilemap_cols; x++) {
            
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, y * tilemap_cols + x, tile_width, tile_height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data + (y * tile_width * width + x * tile_width) * 4);
            count++;
        }
    }
    
    //printf("cnt: %d\n", count);
    glPixelStorei(GL_UNPACK_ROW_LENGTH,   0);
    glPixelStorei(GL_UNPACK_IMAGE_HEIGHT, 0);
    
    fail_unless(glGetError() == GL_NO_ERROR, "");
    
    return GLTilemapTexture{tilemap_id, width, height, tilemap_rows, tilemap_cols};
}

// NOTE: maybe I should get rid of glm, since its only used here...
internal void gl_slow_tilemap_draw(GLTilemapTexture const* tm, fvec2 _pos, fvec2 _size, float rotate, i32 tm_index, b32 mirrorx, b32 mirrory, NRGBA tint, b32 account_for_offset) {
    if (tm_index < 0) return;
    glBindTexture(GL_TEXTURE_2D_ARRAY, tm->texture_id);
    glBindVertexArray(g_quad_vao);
    glUseProgram(g_shd_2d.id);
    
    GLuint layer_loc = glGetUniformLocation(g_shd_2d.id, "layer");
    glUniform1i(layer_loc,tm_index );
    
    glm::vec2 pos = {_pos.x, _pos.y};
    glm::vec2 size = {_size.x, _size.y};
    
    glm::mat4 model(1.0f);
    
    model = glm::scale(model, glm::vec3(g_pixel_scale, g_pixel_scale, 1.f));
    // ORIGIN on BOTTOM-CENTER
    //model = glm::translate(model, glm::vec3(pos - glm::vec2(0, size.y), 0.0f));
    // ORIGIN on TOP-LEFT
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    
    // NOTE(mdodis): translate half width inwards, and full width outwards
    // to keep space for the textures on the sides, should probably color them
    // as well
    if (account_for_offset)
        model = glm::translate(model, glm::vec3(32.f, 64.f, 0.f));
    
    model = glm::translate(model, glm::vec3(0.5f * size.x, .5*size.y, 0.0f));
    model = glm::rotate(model, glm::radians(rotate), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::translate(model, glm::vec3(-0.5f * size.x,-.5*size.y, 0.0f));
    
    if (mirrory) {
        model = glm::translate(model, glm::vec3(0, 1.f * size.y, 0.0f));
        model = glm::scale(model, glm::vec3(1.0f, -1.f,1.f));
    }
    
    if (mirrorx) {
        model = glm::translate(model, glm::vec3(1.f* size.x, 0, 0.0f));
        model = glm::scale(model, glm::vec3(-1.0f, 1.f,1.f));
    }
    
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    GLuint loc = glGetUniformLocation(g_shd_2d.id, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
    loc = glGetUniformLocation(g_shd_2d.id, "tint");
    glUniform4f(loc, tint.r, tint.g, tint.b, tint.a);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

internal void gl_background_draw() {
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, g_background_texture_id);
    
    glUseProgram(g_shd_bg.id);
    
    glm::mat4 model(1.f);
    glm::vec2 size = glm::vec2(960, 768);
    
    model = glm::scale(model, glm::vec3(g_pixel_scale, g_pixel_scale, 1.f));
    model = glm::translate(model, glm::vec3(32.f, 64.f, 0.f));
    model = glm::scale(model, glm::vec3(size, 1.0f));
    
    GLuint layer_loc = glGetUniformLocation(g_shd_bg.id, "sampler");
    glUniform1i(layer_loc, 0);
    
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        error("BB err %x\n",err);
        exit(0);
    }
    
    GLuint loc = glGetUniformLocation(g_shd_bg.id, "model");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
    
    loc = glGetUniformLocation(g_shd_bg.id, "projection");
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(g_projection));
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(g_shd_2d.id);
    
}

internal void gl_init() {
    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
    
    g_shd_bg.create(g_2d_vs, g_bg_fs);
    g_shd_2d.create(g_2d_vs, g_2d_fs);
    
    GLuint sampler_loc = glGetUniformLocation(g_shd_2d.id, "sampler");
    glUniform1i(sampler_loc, 0);
    
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
    glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    glBindVertexArray(g_quad_vao);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    glGenTextures(1, &g_background_texture_id);
    gl_load_background_texture(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    
    
    assert(glGetError() == GL_NO_ERROR);
    
}
