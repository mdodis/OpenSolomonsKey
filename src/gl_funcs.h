#ifndef OSK_GL_FUNCS
#define OSK_GL_FUNCS

#define DEF_GLFUNC(rett, pfnname, actual, ...) typedef rett(*PFNGL##pfnname)(__VA_ARGS__); internal PFNGL##pfnname gl##actual;

DEF_GLFUNC(void,     TEXSTORAGE3D,              TexStorage3D,               GLenum target,GLsizei levels,GLenum internalformat,GLsizei width,GLsizei height,GLsizei depth);
DEF_GLFUNC(GLuint,   CREATESHADER,              CreateShader,               GLenum shaderType);
DEF_GLFUNC(void,     SHADERSOURCE,              ShaderSource,               GLuint shader, GLsizei count,const GLchar * const *string,const GLint *length);
DEF_GLFUNC(void,     COMPILESHADER,             CompileShader,              GLuint shader);
DEF_GLFUNC(void,     GETSHADERIV,               GetShaderiv,                GLuint shader,GLenum pname,GLint *params);
DEF_GLFUNC(void,     GETSHADERINFOLOG,          GetShaderInfoLog,           GLuint shader,GLsizei maxLength,GLsizei *length,GLchar *infoLog);
DEF_GLFUNC(GLuint,   CREATEPROGRAM,             CreateProgram,              void);
DEF_GLFUNC(void,     ATTACHSHADER,              AttachShader,               GLuint program,GLuint shader);
DEF_GLFUNC(void,     LINKPROGRAM,               LinkProgram,                GLuint program);
DEF_GLFUNC(void,     GETPROGRAMIV,              GetProgramiv,               GLuint program, GLenum pname, GLint *params);
DEF_GLFUNC(void,     GETPROGRAMINFOLOG,         GetProgramInfoLog,          GLuint program,GLsizei maxLength,GLsizei *length,GLchar *infoLog);
DEF_GLFUNC(void,     USEPROGRAM,                UseProgram,                 GLuint program);
DEF_GLFUNC(void,     DELETESHADER,              DeleteShader,               GLuint shader);
DEF_GLFUNC(GLint,    GETUNIFORMLOCATION,        GetUniformLocation,         GLuint program,const GLchar * name);
DEF_GLFUNC(void,     UNIFORM1I,                 Uniform1i,                  GLint location, GLint v0);
DEF_GLFUNC(void,     GENVERTEXARRAYS,           GenVertexArrays,            GLsizei n,GLuint *arrays);
DEF_GLFUNC(void,     GENBUFFERS,                GenBuffers,                 GLsizei n,GLuint * buffers);
DEF_GLFUNC(void,     BINDBUFFER,                BindBuffer,                 GLenum target,GLuint buffer);
DEF_GLFUNC(void,     BUFFERDATA,                BufferData,                 GLenum target,GLsizeiptr size,const GLvoid * data,GLenum usage);
DEF_GLFUNC(void,     BINDVERTEXARRAY,           BindVertexArray,            GLuint array);
DEF_GLFUNC(void,     ENABLEVERTEXATTRIBARRAY,   EnableVertexAttribArray,    GLuint index);
DEF_GLFUNC(void,     DISABLEVERTEXATTRIBARRAY,  DisableVertexAttribArray,   GLuint index);
DEF_GLFUNC(void,     VERTEXATTRIBPOINTER,       VertexAttribPointer,        GLuint index,GLint size,GLenum type,GLboolean normalized,GLsizei stride,const GLvoid * pointer);
DEF_GLFUNC(void,     UNIFORMMATRIX4FV,          UniformMatrix4fv,           GLint location,GLsizei count,GLboolean transpose,const GLfloat * value);

#if defined(OSK_PLATFORM_X11)

#define LOAD_GLFUNC(actual,pfnname) gl##actual = (PFNGL##pfnname)glXGetProcAddress((const GLubyte*)#actual)

internal void
gl_load()
{
    LOAD_GLFUNC(TexStorage3D,               TEXSTORAGE3D);
    LOAD_GLFUNC(CreateShader,               CREATESHADER);
    LOAD_GLFUNC(ShaderSource,               SHADERSOURCE);
    LOAD_GLFUNC(CompileShader,              COMPILESHADER);
    LOAD_GLFUNC(GetShaderiv,                GETSHADERIV);
    LOAD_GLFUNC(GetShaderInfoLog,           GETSHADERINFOLOG);
    LOAD_GLFUNC(CreateProgram,              CREATEPROGRAM);
    LOAD_GLFUNC(AttachShader,               ATTACHSHADER);
    LOAD_GLFUNC(LinkProgram,                LINKPROGRAM);
    LOAD_GLFUNC(GetProgramiv,               GETPROGRAMIV);
    LOAD_GLFUNC(GetProgramInfoLog,          GETPROGRAMINFOLOG);
    LOAD_GLFUNC(UseProgram,                 USEPROGRAM);
    LOAD_GLFUNC(DeleteShader,               DELETESHADER);
    LOAD_GLFUNC(GetUniformLocation,         GETUNIFORMLOCATION);
    LOAD_GLFUNC(Uniform1i,                  UNIFORM1I);
    LOAD_GLFUNC(GenVertexArrays,            GENVERTEXARRAYS);
    LOAD_GLFUNC(GenBuffers,                 GENBUFFERS);
    LOAD_GLFUNC(BindBuffer,                 BINDBUFFER);
    LOAD_GLFUNC(BufferData,                 BUFFERDATA);
    LOAD_GLFUNC(BindVertexArray,            BINDVERTEXARRAY);
    LOAD_GLFUNC(EnableVertexAttribArray,    ENABLEVERTEXATTRIBARRAY);
    LOAD_GLFUNC(DisableVertexAttribArray,   DISABLEVERTEXATTRIBARRAY);
    LOAD_GLFUNC(VertexAttribPointer,        VERTEXATTRIBPOINTER);
    LOAD_GLFUNC(UniformMatrix4fv,           UNIFORMMATRIX4FV);
}

#elif defined(OSK_PLATFORM_WIN32)

#else
#error "No platform Opengl load proc defined in gl_funcs.h!"
#endif


#undef LOAD_GLFUNC
#undef DEF_GLFUNC

#endif //! OSK_GL_FUNCS
