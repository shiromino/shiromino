#include "Video/Screen.h"
#include "Debug.h"
#include <iostream>

using namespace Shiro;
using namespace std;

#ifdef ENABLE_OPENGL_INTERPOLATION
static GLuint CompileShader(GLenum shaderType, const GLchar* shaderSource);
static GLuint CompileShadingProgram(const GLchar* vertexShaderSource, const GLchar* geometryShaderSource, const GLchar* fragmentShaderSource);
#endif

Screen::Screen(const std::string& name, const unsigned w, const unsigned h, const float render_scale) :
    name(name),
    w(w),
    h(h),
    render_scale(render_scale),
    window(nullptr),
    renderer(nullptr),
    target_tex(nullptr)
#ifdef ENABLE_OPENGL_INTERPOLATION
    , interpolate_shading_prog(0u)
#endif
{}

Screen::~Screen() {
#ifdef ENABLE_OPENGL_INTERPOLATION
    if (target_tex) {
        SDL_DestroyTexture(target_tex);
    }
    if (interpolate_shading_prog) {
        glDeleteProgram(interpolate_shading_prog);
    }
#endif

    if (renderer) {
        SDL_DestroyRenderer(renderer);
    }

    if (window) {
        SDL_DestroyWindow(window);
    }
}

bool Screen::init(const Settings& settings) {
    Uint32 windowFlags = SDL_WINDOW_RESIZABLE;
#ifdef ENABLE_OPENGL_INTERPOLATION
    // TODO: Figure out whether OpenGL 2.0 should be used for desktop. Also
    // support OpenGL ES 2.0.
    if (settings.interpolate) {
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
        windowFlags |= SDL_WINDOW_OPENGL;
    }
#endif
    window = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, windowFlags);
    if (window == nullptr) {
        log_err("SDL_CreateWindow: %s", SDL_GetError());
        return false;
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE | (settings.vsync ? SDL_RENDERER_PRESENTVSYNC : 0));
    if (renderer == nullptr) {
        log_err("SDL_CreateRenderer: %s", SDL_GetError());
        SDL_DestroyWindow(window);
        return false;
    }

    if (SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND) < 0) {
        log_err("SDL_SetRenderDrawBlendMode: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }

    SDL_SetWindowMinimumSize(window, 640, 480);
    if (settings.fullscreen) {
        SDL_SetWindowSize(window, 640, 480);
        if (SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN) < 0) {
            log_err("SDL_SetWindowFullscreen: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }

    /*if (SDL_RenderSetLogicalSize(renderer, 640, 480) < 0) {
        log_err("SDL_RenderSetLogicalSize: %s", SDL_GetError());
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        return false;
    }*/

    if (!settings.videoStretch) {
        if (SDL_RenderSetIntegerScale(renderer, SDL_TRUE) < 0) {
            log_err("SDL_RenderSetIntegerScale: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }

#ifdef ENABLE_OPENGL_INTERPOLATION
    if (settings.interpolate) {
        target_tex = SDL_CreateTexture(renderer, SDL_GetWindowPixelFormat(window),  SDL_TEXTUREACCESS_TARGET, static_cast<int>(640.0 * render_scale), static_cast<int>(480.0 * render_scale));
        if (target_tex == nullptr) {
            log_err("SDL_CreateTexture: %s", SDL_GetError());
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }
    else {
        target_tex = NULL;
    }

    interpolate_shading_prog = 0u;

    if (settings.interpolate) {
        if (!gladLoadGL()) {
            log_err("gladLoadGL failed");
            SDL_DestroyTexture(target_tex);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);

        /*
        Vertex layout (triangle strip):

        0 - 2
        | / |
        1 - 3
        */

        // By avoiding as much client-to-server data transfer as possible,
        // performance should be better; memory access on modern systems is much
        // more expensive than just generating the data with computation, in many
        // cases. So just calculate positions from gl_VertexID, rather than using
        // vertex arrays. And take advantage of hardware linear interpolation,
        // rather than sampling in the fragment shader multiple times and doing
        // our own interpolation.
        interpolate_shading_prog = CompileShadingProgram(
            "#version 330\n"
            "out vec2 texCoord;\n"
            "void main() {\n"
            "   vec4 pos = vec4(float(gl_VertexID / 2 != 0) * 2 - 1, float(gl_VertexID % 2 == 0) * 2 - 1, 0.0, 1.0);\n"
            "   gl_Position = pos;\n"
            "   texCoord = (pos.xy + vec2(1.0, -1.0)) / vec2(2.0, -2.0);\n"
            "}\n",

            NULL,

            "#version 330\n"
            "in vec2 texCoord;\n"
            "uniform sampler2D tex;\n"
            "uniform vec2 viewportSize;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "const vec2 texSize = vec2(640.0, 480.0);\n"

            // filterCoord provided by PARTY MAN X ( https://github.com/PARTYMANX ).
            // It's better than the thing I hacked together before. -Brandon McGriff
            "vec2 filterCoord(vec2 uv, vec2 srcRes) {\n"
                "vec2 invSrc = 1.0 / srcRes;\n"

                // calculate destination resolution
                "vec2 duv = vec2(dFdx(uv.x), dFdy(uv.y));\n"
                "vec2 dstRes = 1.0 / abs(duv);\n"

                "vec2 scale = dstRes * invSrc;\n"                   // (dstRes / invSrc)

                "vec2 scaleFactor = floor(scale) + 1.0;\n"          // add one to integer scale factor so we scale down, not up

                "vec2 texelCoord = uv * srcRes;\n"                  // coordinate in texel space

                "vec2 roundCoord = floor(texelCoord) + 0.5;\n"      // center of nearest texel of uv
                "vec2 fractCoord = fract(texelCoord) - 0.5;\n"      // offset from nearest texel

                "vec2 offset = abs(fractCoord * scaleFactor);\n"    // absolute offset multiplied by scale factor
                "offset -= 0.5 * (scaleFactor - 1.0);\n"            // subtract border of scaled texel
                "offset = max(offset, 0.0) * sign(fractCoord);\n"   // get rid of wrong direction offsets, restore sign

                "return (roundCoord + offset) * invSrc;\n"
            "}\n"

            "void main() {\n"
            "   outColor = texture(tex, filterCoord(texCoord, texSize));\n"
            "}\n");
        if (interpolate_shading_prog == 0u) {
            log_err("CompileShadingProgram failed");
            SDL_DestroyTexture(target_tex);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }

        glUseProgram(interpolate_shading_prog);
        glUniform1i(glGetUniformLocation(interpolate_shading_prog, "tex"), 0);
        glUseProgram(0u);

        if (SDL_GL_BindTexture(target_tex, NULL, NULL) < 0) {
            log_err("SDL_GL_BindTexture: %s", SDL_GetError());
            glDeleteProgram(interpolate_shading_prog);
            SDL_DestroyTexture(target_tex);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        if (SDL_GL_UnbindTexture(target_tex) < 0) {
            log_err("SDL_GL_UnbindTexture: %s", SDL_GetError());
            glDeleteProgram(interpolate_shading_prog);
            SDL_DestroyTexture(target_tex);
            SDL_DestroyRenderer(renderer);
            SDL_DestroyWindow(window);
            return false;
        }
    }
#endif
    return true;
}

#ifdef ENABLE_OPENGL_INTERPOLATION
static GLuint CompileShader(GLenum shaderType, const GLchar* shaderSource) {
    GLint compileOK;
    GLuint shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, &shaderSource, NULL);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileOK);
    if (compileOK) {
        return shader;
    }
    else {
        GLint infoLogLength;
        char* infoLog;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
        infoLog = new char[infoLogLength];
        glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
        const auto shaderTypeString = shaderType == GL_VERTEX_SHADER
            ? "vertex"
            : shaderType == GL_GEOMETRY_SHADER
                ? "geometry"
                : shaderType == GL_FRAGMENT_SHADER
                    ? "fragment"
                    : "unknown";
        log_err("Could not compile %s shader", shaderTypeString);
        log_err("%s", infoLog);
        delete[] infoLog;
        return 0u;
    }
}

static GLuint CompileShadingProgram(const GLchar* vertexShaderSource, const GLchar* geometryShaderSource, const GLchar* fragmentShaderSource) {
    GLuint vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    if (vertexShader == 0u) {
        return 0u;
    }

    GLuint geometryShader = 0u;
    if (geometryShaderSource != NULL) {
        geometryShader = CompileShader(GL_GEOMETRY_SHADER, geometryShaderSource);
        if (geometryShader == 0u) {
            glDeleteShader(vertexShader);
            return 0u;
        }
    }

    GLuint fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);
    if (fragmentShader == 0u) {
        if (geometryShader != 0u) glDeleteShader(geometryShader);
        glDeleteShader(vertexShader);
        return 0u;
    }

    GLuint program = glCreateProgram();
    if (program == 0u) {
        log_err("Could not create shading program");
        glDeleteShader(vertexShader);
        if (geometryShader != 0u) glDeleteShader(geometryShader);
        glDeleteShader(fragmentShader);
        return 0u;
    }

    glAttachShader(program, vertexShader);
    if (geometryShader != 0u) glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);

    glLinkProgram(program);

    glDetachShader(program, vertexShader);
    if (geometryShader != 0u) glDetachShader(program, geometryShader);
    glDetachShader(program, fragmentShader);

    glDeleteShader(vertexShader);
    if (geometryShader != 0u) glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    GLint linkOK;
    glGetProgramiv(program, GL_LINK_STATUS, &linkOK);
    if (linkOK) {
        return program;
    }
    else {
        GLint infoLogLength;
        char* infoLog;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
        infoLog = new char[infoLogLength];
        glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
        log_err("Could not link shading program");
        log_err("%s", infoLog);
        delete[] infoLog;
        glDeleteProgram(program);
        return 0u;
    }
}
#endif
