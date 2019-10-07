#ifndef OSK_GL_FUNCS
#define OSK_GL_FUNCS

#if defined(OSK_PLATFORM_WIN32)
typedef char GLchar;
typedef GLsizei* GLsizeiptr;
#endif

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
DEF_GLFUNC(void,     UNIFORM4F,                 Uniform4f,                  GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);

#define GL_LOAD_BLOCK \
LOAD_GLFUNC(TexStorage3D,               TEXSTORAGE3D);\
LOAD_GLFUNC(CreateShader,               CREATESHADER);\
LOAD_GLFUNC(ShaderSource,               SHADERSOURCE);\
LOAD_GLFUNC(CompileShader,              COMPILESHADER);\
LOAD_GLFUNC(GetShaderiv,                GETSHADERIV);\
LOAD_GLFUNC(GetShaderInfoLog,           GETSHADERINFOLOG);\
LOAD_GLFUNC(CreateProgram,              CREATEPROGRAM);\
LOAD_GLFUNC(AttachShader,               ATTACHSHADER);\
LOAD_GLFUNC(LinkProgram,                LINKPROGRAM);\
LOAD_GLFUNC(GetProgramiv,               GETPROGRAMIV);\
LOAD_GLFUNC(GetProgramInfoLog,          GETPROGRAMINFOLOG);\
LOAD_GLFUNC(UseProgram,                 USEPROGRAM);\
LOAD_GLFUNC(DeleteShader,               DELETESHADER);\
LOAD_GLFUNC(GetUniformLocation,         GETUNIFORMLOCATION);\
LOAD_GLFUNC(Uniform1i,                  UNIFORM1I);\
LOAD_GLFUNC(GenVertexArrays,            GENVERTEXARRAYS);\
LOAD_GLFUNC(GenBuffers,                 GENBUFFERS);\
LOAD_GLFUNC(BindBuffer,                 BINDBUFFER);\
LOAD_GLFUNC(BufferData,                 BUFFERDATA);\
LOAD_GLFUNC(BindVertexArray,            BINDVERTEXARRAY);\
LOAD_GLFUNC(EnableVertexAttribArray,    ENABLEVERTEXATTRIBARRAY); \
LOAD_GLFUNC(DisableVertexAttribArray,   DISABLEVERTEXATTRIBARRAY);\
LOAD_GLFUNC(VertexAttribPointer,        VERTEXATTRIBPOINTER); \
LOAD_GLFUNC(UniformMatrix4fv,           UNIFORMMATRIX4FV); \
LOAD_GLFUNC(Uniform4f,                  UNIFORM4F); \

#if defined(OSK_PLATFORM_X11)

#define LOAD_GLFUNC(actual,pfnname) gl##actual = (PFNGL##pfnname)glXGetProcAddress((const GLubyte*)#actual)

internal void
gl_load()
{
    GL_LOAD_BLOCK
}

#elif defined(OSK_PLATFORM_WIN32)

void *win32_gl_proc_address( const char *name)
{
    
    void* p = wglGetProcAddress(name);
    OutputDebugStringA(name);
    fail_unless(p, "failed to get proc address for gl function!");
    return p;
}

#define _oSTR(x) #x
#define oSTR(x) _oSTR(x)

#define LOAD_GLFUNC(actual,pfnname) gl##actual = (PFNGL##pfnname)win32_gl_proc_address((const char*) oSTR(gl##actual))

// For some reason OpenGL on Windows doesn't have these functions loaded
// (even if they exist in OpenGL 1.2), and declaring them with glx fails because
// they _are_ included there, so special case for Win32. Yay
DEF_GLFUNC(void,     TEXSUBIMAGE3D,             TexSubImage3D,              GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width,GLsizei height,GLsizei depth,GLenum format,GLenum type, const GLvoid* data);
DEF_GLFUNC(void,     ACTIVETEXTURE,             ActiveTexture,              GLenum texture);

#define WIN32_GL_EXTRA \
LOAD_GLFUNC(TexSubImage3D,              TEXSUBIMAGE3D); \
LOAD_GLFUNC(ActiveTexture,              ACTIVETEXTURE); \

internal void
gl_load()
{
    GL_LOAD_BLOCK
        WIN32_GL_EXTRA
}
#else
#error "No platform Opengl load proc defined in gl_funcs.h!"
#endif

#define GL_TEXTURE_2D                     0x0DE1
#define GL_TEXTURE_2D_ARRAY               0x8C1A
#define GL_FRAGMENT_SHADER                0x8B30
#define GL_VERTEX_SHADER                  0x8B31
#define GL_COMPILE_STATUS                 0x8B81
#define GL_LINK_STATUS                    0x8B82
#define GL_NEAREST                        0x2600
#define GL_LINEAR                         0x2601
#define GL_NEAREST_MIPMAP_NEAREST         0x2700
#define GL_LINEAR_MIPMAP_NEAREST          0x2701
#define GL_NEAREST_MIPMAP_LINEAR          0x2702
#define GL_LINEAR_MIPMAP_LINEAR           0x2703
#define GL_TEXTURE_MAG_FILTER             0x2800
#define GL_TEXTURE_MIN_FILTER             0x2801
#define GL_TEXTURE_WRAP_S                 0x2802
#define GL_TEXTURE_WRAP_T                 0x2803
#define GL_REPEAT                         0x2901
#define GL_CLAMP_TO_EDGE                  0x812F
#define GL_RGBA8                          0x8058
#define GL_RGBA                           0x1908
#define GL_UNSIGNED_BYTE                  0x1401
#define GL_TEXTURE_BASE_LEVEL             0x813C
#define GL_TEXTURE_MAX_LEVEL              0x813D
#define GL_UNPACK_ROW_LENGTH              0x0CF2
#define GL_UNPACK_IMAGE_HEIGHT            0x806E
#define GL_TEXTURE0                       0x84C0
#define GL_ARRAY_BUFFER                   0x8892
#define GL_STATIC_DRAW                    0x88E4

#undef LOAD_GLFUNC
#undef DEF_GLFUNC

#endif //! OSK_GL_FUNCS
