// ngotrung Sun 22 Aug 2021 09:46:58 AM +07
#ifndef GRAPHICS_SPRITE_RENDERER_H
#define GRAPHICS_SPRITE_RENDERER_H
#include "graphics/material.h"
#include "graphics/types.h"
#include "graphics/vertex_buffer.h"
#include "toolbox/common.h"


/* sort by drawOrder -> sprite->texture -> material */

void sprite_renderer_init(u32 buffer_size);
void sprite_renderer_shutdown(void);

/**
 * \brief prepare buffer for rendering
 */
void begin_sprite(mat4 viewProjMat);

/**
 * \brief submit all draw commands to renderer
 */
void end_sprite(void);

/**
 * \brief draw sprite simple version
 */
void draw_sprite(const Sprite* sprite, const Material* material, vec2 position, vec4 color);

void draw_sprite_ex(const Sprite*   sprite,
                    const Material* material,
                    mat3            transform,
                    vec2            center,
                    vec4            color,
                    bool            vertFlip,
                    bool            horiFlp);

#endif // GRAPHICS_SPRITE_RENDERER_H
