#ifndef GL_H
#define GL_H

#if defined(_WIN32)

#include "glad/gl.h"

#else

#define GL_GLEXT_PROTOTYPES
#include "GL/gl.h"
#include "SDL_opengl.h"
#endif

#endif //! GL_H
