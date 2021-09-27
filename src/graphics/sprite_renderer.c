#include "graphics/sprite_renderer.h"

typedef struct DrawCmd
{
    const Material* material;
    const Texture*  texture;
    Vertex          vertices[4];
    int             z_order;
    int             sorting_layer;
} DrawCmd;

static const char s_dft_vert_shader_src[] = /**/
    "#version 330 core\n"
    "layout (location = 0) in vec3 a_Position;"
    "layout (location = 1) in vec2 a_TexCoords;"
    "layout (location = 2) in vec4 a_Color;"
    "out vec4 vertColor;"
    "out vec2 texCoord;"
    "uniform mat4 u_viewProjectionMatrix;"

    "void main()"
    "{"
    "    gl_Position =  u_viewProjectionMatrix * vec4(a_Position.x, a_Position.y, 0.f, 1.0);\n"
    "    gl_Position.z = 0.f;"
    "    vertColor = a_Color;"
    "    texCoord = a_TexCoords;"
    "}";

static const char s_dft_frag_shader_src[] = /**/
    "#version 330 core\n"
    "in vec4 vertColor;"
    "in vec2 texCoord;"
    "uniform sampler2D tex;"
    "out vec4 fragColor;"
    "void main() {"
    "    fragColor = texture(tex, texCoord) * vertColor;"
    "    if (fragColor.a < 0.1f)"
    "       discard;"
    "}";

static DrawCmd*         s_commands;
static u32              s_count;
static u32              s_size;
static Texture          s_dft_texture;
static Material         s_dft_material;
static mat4             s_used_view_proj_matrix;
static VertexBuffer     s_vert_buf;
static Shader           s_dft_shader;
static RenderStatistics s_statistics;

static int
compare_pointer(const void* lhs, const void* rhs)
{
    const u8* a = lhs;
    const u8* b = rhs;

    return (a > b) - (b > a);
}

static int
compare_draw_command(const void* lhs, const void* rhs)
{
    const DrawCmd* cmd1 = lhs;
    const DrawCmd* cmd2 = rhs;

    if (cmd1->sorting_layer != cmd2->sorting_layer)
        return cmd1->sorting_layer - cmd2->sorting_layer;

    if (cmd1->z_order != cmd2->z_order)
        return cmd1->z_order - cmd2->z_order;

    if (cmd1->texture != cmd2->texture)
        return compare_pointer(cmd1->texture, cmd2->texture);

    return compare_pointer(cmd1->material, cmd2->material);
}

static void
submit_draw_call(void)
{
    s_statistics.draw_call_count++;
    s_statistics.vertex_count += s_vert_buf.count;
    draw_vertex_buffer(&s_vert_buf);
}

static void
check_vertex_buffer_limit(u32 needed_space)
{
    if (needed_space > vertex_buffer_availiable(&s_vert_buf))
        submit_draw_call();
}

static void
submit_internal(void)
{
    if (s_count == 0)
        return;

    /* sort draw command base on it's material, texture and draw order */
    SDL_qsort(s_commands, s_count, sizeof *s_commands, compare_draw_command);
    const DrawCmd*       cmd;
    const DrawCmd* const end      = &s_commands[s_count];
    const DrawCmd* const begin    = &s_commands[0];
    const Material*      material = begin->material;
    const Texture*       texture  = begin->texture;
    material_use(material);

    /* TODO: use uniform buffer object instead */
    shader_upload_view_project_matrix(material->shader, s_used_view_proj_matrix);

    texture_bind(texture);

    vertex_buffer_rewind(&s_vert_buf);
    for (cmd = begin; cmd != end; ++cmd)
    {
        if (material != cmd->material) /* force submit draw call if using diffrent material */
        {
            submit_draw_call();

            /* switch material */
            material = cmd->material;
            material_use(material);
            shader_upload_view_project_matrix(material->shader, s_used_view_proj_matrix);
        }

        if (texture != cmd->texture) /* force submit draw call if using diffrent texture */
        {
            submit_draw_call();

            /* switch texture */
            texture = cmd->texture;
            texture_bind(texture);
        }

        check_vertex_buffer_limit(4);
        vertex_buffer_push_nvertex(&s_vert_buf, cmd->vertices, 4);
    }
    submit_draw_call();
    s_count = 0;
}

void
sprite_renderer_init(u32 size)
{
    s_size     = size;
    s_count    = 0;
    s_commands = SDL_malloc(size * (sizeof *s_commands));
    vertex_buffer_init(&s_vert_buf, 2048);

    u32 white = 0xffffffff;
    texture_load_from_memory(&s_dft_texture, (u8*)&white, 1, 1, PIXEL_FORMAT_RGBA);

    ASSERT(shader_load_from_source(&s_dft_shader, s_dft_vert_shader_src, s_dft_frag_shader_src) == 0);

    material_init(&s_dft_material, &s_dft_shader, NULL, 0);

    s_statistics.draw_call_count = 0;
    s_statistics.vertex_count    = 0;
}

void
sprite_renderer_shutdown()
{

    SDL_free(s_commands);
    s_commands = NULL;
    vertex_buffer_destroy(&s_vert_buf);

    texture_destroy(&s_dft_texture);
    shader_destroy(&s_dft_shader);
}

void
begin_sprite(mat4 view_projection_matrix)
{
    glm_mat4_copy(view_projection_matrix, s_used_view_proj_matrix);
    s_count                      = 0;
    s_statistics.draw_call_count = 0;
    s_statistics.vertex_count    = 0;
}

void
end_sprite(void)
{
    submit_internal();
}

INLINE u32
draw_command_buffer_availiable()
{
    return s_size - s_count;
}

INLINE void
check_command_buffer_limit(u32 needed_space)
{
    if (draw_command_buffer_availiable() < needed_space)
        submit_internal();
}

void
draw_sprite(const Sprite* sprite, const Material* material, vec2 position, vec4 color)
{
    const float    w = sprite ? (float)sprite->rect.w : 16.f;
    const float    h = sprite ? (float)sprite->rect.h : 16.f;
    DrawCmd*       cmd;
    Vertex*        vert;
    float          u1, v1, u2, v2;
    const Texture* texture;

    check_command_buffer_limit(1);
    cmd  = &s_commands[s_count++];
    vert = &cmd->vertices[0];

    if (sprite == NULL || sprite->texture == NULL)
    {
        texture = &s_dft_texture;
        u1      = 0.f;
        v1      = 0.f;
        u2      = 1.f;
        v2      = 1.f;
    }
    else
    {
        texture = sprite->texture;
        u1      = sprite->u1;
        v1      = sprite->v1;
        u2      = sprite->u2;
        v2      = sprite->v2;
    }

    if (material == NULL)
    {
        material = &s_dft_material;
    }

    vert->position[0]   = position[0];
    vert->position[1]   = position[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // top-right corner
    vert->position[0]   = position[0] + w;
    vert->position[1]   = position[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-right corner
    vert->position[0]   = position[0] + w;
    vert->position[1]   = position[1] + h;
    vert->position[2]   = 0;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-left corner
    vert->position[0]   = position[0];
    vert->position[1]   = position[1] + h;
    vert->position[2]   = 0;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);

    cmd->material      = material;
    cmd->texture       = texture;
    cmd->z_order       = 0;
    cmd->sorting_layer = 0;
}

void
draw_sprite_ex(const Sprite*   sprite,
               const Material* material,
               mat3            transform_matrix,
               vec2            center,
               vec4            color,
               bool            vert_flip,
               bool            hori_flip)
{

    float          u1, v1, u2, v2; /* texture coordinates                          */
    Vertex*        vert;           /* begin of cmd's vertices                      */
    vec3           local_pos;      /* vertex position in model space               */
    const Texture* texture;        /* texture to use                               */
    DrawCmd*       cmd;
    const float    w = sprite ? (float)sprite->rect.w : 16.f;
    const float    h = sprite ? (float)sprite->rect.h : 16.f;

    check_command_buffer_limit(1);
    cmd  = &s_commands[s_count++];
    vert = &cmd->vertices[0];

    if (sprite == NULL || sprite->texture == NULL)
    {
        texture = &s_dft_texture;
        u1      = 0.f;
        v1      = 0.f;
        v2      = 1.f;
        u2      = 1.f;
    }
    else
    {
        texture = sprite->texture;
        if (hori_flip)
        {
            u1 = sprite->u2;
            u2 = sprite->u1;
        }
        else
        {
            u1 = sprite->u1;
            u2 = sprite->u2;
        }
        if (vert_flip)
        {
            v1 = sprite->v2;
            v2 = sprite->v1;
        }
        else
        {
            v1 = sprite->v1;
            v2 = sprite->v2;
        }
    }

    if (material == NULL)
    {
        material = &s_dft_material;
    }

    // top-left corner
    local_pos[0] = 0.f - center[0];
    local_pos[1] = 0.f - center[1];
    local_pos[2] = 1.f;
    glm_mat3_mulv(transform_matrix, local_pos, vert->position);

    vert->position[2]   = 0.f;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // top-right corner
    local_pos[0] = w - center[0];
    local_pos[1] = 0.f - center[1];
    local_pos[2] = 1.f;
    glm_mat3_mulv(transform_matrix, local_pos, vert->position);

    vert->position[2]   = 0.f;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-right corner
    local_pos[0] = w - center[0];
    local_pos[1] = h - center[1];
    local_pos[2] = 1.f;
    glm_mat3_mulv(transform_matrix, local_pos, vert->position);

    vert->position[2]   = 0.f;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-left corner
    local_pos[0] = 0.f - center[0];
    local_pos[1] = h - center[1];
    local_pos[2] = 1.f;
    glm_mat3_mulv(transform_matrix, local_pos, vert->position);

    vert->position[2]   = 0.f;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);

    cmd->z_order  = 0;
    cmd->texture  = texture;
    cmd->material = material;
}

void draw(const struct Vertex*   vertices,
          u32                    count,
          enum Primitive         primitive,
          const struct Texture*  texture,
          const struct Material* material,
          const mat4             model_matrix,
          s32                    priority);
