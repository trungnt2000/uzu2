// ngotrung Wed 16 Jun 2021 08:16:56 PM +07
#ifndef GRAPHICS
#define GRAPHICS
#include "graphics/types.h" /* basic types */
#include "graphics/renderer.h"
#include "graphics/font.h"
#include "graphics/view.h"
#include "graphics/sprite_sheet.h"
#include "graphics/animation.h"
/**
 * helper function to create new shader program
 * with given vertex shader and fragment shader file
 * output program is set via outProgram param
 * \return 0 if succeed -1 otherwise
 */
int create_shader_form_file(const char* vsFile, const char* fsFile, unsigned int* shaderReturn);

int create_shader_form_sources(const char* vsSource, const char* fsSource, unsigned int* shaderReturn);

#endif // GRAPHICS
