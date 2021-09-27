#include "graphics/text_renderer.h"
#include "cglm/cglm.h"
#include "graphics/gl.h"
#include "toolbox.h"

/* wheather or not drawing */
static bool s_is_drawing;

/* EBO */
static GLuint s_ebo;

/* VBO (position, uv, color)*/
static GLuint s_vbo;

/* VAO */
static GLuint s_vao;

/* vertex buffer*/
static Vertex* s_vert_buf;

/* next vertex pointer in vertex buffer */
static Vertex* s_next_vert_ptr;

/* how many sprite are stored in buffer */
static u32 s_count;

/* how many sprite we can batch together in one draw call */
static u32 s_buffer_size;

/* draw call count between begin and end call */
static u32 s_draw_call_cnt;

static u32 s_vert_cnt;

static Font s_dft_font;

static Shader s_dft_shader;

static const Font*   s_used_font;
static const Shader* s_used_shader;
static mat4          s_used_mvp_matrix;

static void submit(void);
static void check_buffer_limit(u32 needed_space);
static void draw_glyph(const Font* font, u32 codepoint, vec2 position, float scale, vec4 color);

static float
text_width(const char* text, const Font* font, float scale)
{
    float     width = 0;
    const u8* iter  = (const u8*)text;
    u32       codepoint;
    while (*iter != '\0' && *iter != '\n')
    {
        codepoint = (u32)*iter++;
        width += font->glyphs[codepoint].advance[0] * scale;
    }

    return width;
}

static float
align_text(float x, float text_width, TextAlignment alignment)
{
    switch (alignment)
    {
    case TEXT_ALIGN_LEFT:
        return x;
        break;
    case TEXT_ALIGN_RIGHT:
        return x - text_width;
        break;
    case TEXT_ALIGN_CENTER:
        return x - text_width / 2.f;
        break;
    }
}

void
text_renderer_init(u32 buffer_size)
{
    GLsizei vbo_size, ebo_size;

    s_buffer_size   = buffer_size;
    s_count         = 0;
    s_draw_call_cnt = 0;

    /* four vertcies per single sprite */
    vbo_size = (GLsizei)(buffer_size * sizeof(Vertex) * 4);

    /* six indices per single sprite */
    ebo_size = (GLsizei)(buffer_size * sizeof(u32) * 6);

    u32* indices = SDL_malloc((size_t)ebo_size);
    for (u32 i = 0; i < buffer_size; ++i)
    {
        indices[(i * 6) + 0] = (i * 4) + 0;
        indices[(i * 6) + 1] = (i * 4) + 1;
        indices[(i * 6) + 2] = (i * 4) + 3;
        indices[(i * 6) + 3] = (i * 4) + 1;
        indices[(i * 6) + 4] = (i * 4) + 2;
        indices[(i * 6) + 5] = (i * 4) + 3;
    }

    glGenVertexArrays(1, &s_vao);
    glGenBuffers(1, &s_vbo);
    glGenBuffers(1, &s_ebo);

    /*set up vao with one sVbo for position, texture coordinates, color and static
     * a ebo for indices */
    glBindVertexArray(s_vao);

    /*pre-allocate memory for sVbo*/
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferData(GL_ARRAY_BUFFER, vbo_size, NULL, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, s_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, ebo_size, indices, GL_STATIC_DRAW);

    /* store position attribute in vertex attribute list */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
    glEnableVertexAttribArray(0);

    /* store texture coordinates attribute in vertex attribute list */
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(1);

    /* store color attribute in vertex attribute list */
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    s_is_drawing = false;
    s_vert_buf   = SDL_malloc((size_t)vbo_size);

    SDL_free(indices);

    // load default shader, font, texture
    if (font_load(&s_dft_font, "res/font/font.TTF", 32) != 0)
    {
        UZU_ERROR("Failed to load default font\n");
        return;
    }

    if (shader_load(&s_dft_shader, "res/shader/sprite.vert", "res/shader/sprite.frag") != 0)
    {
        UZU_ERROR("Failed to load default shader\n");
        return;
    }
}

void
text_renderer_shutdown()
{
    glDeleteBuffers(1, &s_ebo);
    glDeleteBuffers(1, &s_vbo);
    glDeleteVertexArrays(1, &s_vao);
    SDL_free(s_vert_buf);
}

void
draw_text(const char* text, float x, float y, vec4 color)
{
    draw_text_ex(text, &s_dft_font, x, y, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_textv(const char* text, vec2 position, vec4 color)
{
    draw_textv_ex(text, &s_dft_font, position, 1.f, TEXT_ALIGN_LEFT, color);
}

void
draw_text_ex(const char*   text,
             const Font*   font,
             float         x,
             float         y,
             float         scale,
             TextAlignment alignment,
             vec4          color)
{
    draw_textv_ex(text, font, (vec2){ x, y }, scale, alignment, color);
}

void
draw_textv_ex(const char*   text,
              const Font*   font,
              vec2          position,
              float         scale,
              TextAlignment alignment,
              vec4          color)
{
    // TODO: decode utf8
    vec2      draw_pos;
    const u8* iter;
    u32       codepoint;

    if (font == NULL)
        font = &s_dft_font;

    if (s_used_font != font)
    {
        submit();
        s_used_font = font;
    }

    draw_pos[1] = position[1] + font->max_glyph_height * scale;

    while (text != NULL)
    {
        draw_pos[0] = align_text(position[0], text_width(text, font, scale), alignment);
        iter        = (const u8*)text;

        while (*iter && *iter != '\n')
        {
            codepoint = (u32)*iter++;
            draw_glyph(font, codepoint, draw_pos, scale, color);
            draw_pos[0] += font->glyphs[codepoint].advance[0] * scale;
        }
        text = (const char*)(*iter == '\0' ? NULL : iter + 1);
        draw_pos[1] += font->max_glyph_height * scale;
    }
}

static bool
get_next_line(const char** text_ptr,
              const Font*  font,
              u8*          buf,
              int          buf_size,
              float        width_limit,
              float*       width_return)
{
    float     width = 0;
    int       count = 0;
    const u8* text  = (const u8*)*text_ptr;
    if (text == NULL)
        return false;

    while (*text && *text != '\n' && width < width_limit && count < buf_size - 1)
    {
        width += font->glyphs[(u32)*text].advance[0];
        *buf++ = *text++;
        ++count;
    }
    *buf          = '\0';
    *width_return = width;
    *text_ptr     = (const char*)(*text == '\0' ? NULL : text + 1);

    return count != 0;
}

void
draw_text_boxed_ex(const char*   text,
                   const Font*   font,
                   vec2          position,
                   vec2          size,
                   float         scale,
                   TextAlignment alignment,
                   vec4          color)
{
    static u8   buf[512] = { 0 };
    float       line_width;
    u8*         iter;
    const float row_size = font->max_glyph_height * scale;
    vec2        draw_pos = { [1] = position[1] + row_size };
    u32         codepoint;

    if (font == NULL)
        font = &s_dft_font;

    if (s_used_font != font)
    {
        submit();
        s_used_font = font;
    }

    while (get_next_line(&text, font, buf, 512, size[1] / scale, &line_width))
    {
        iter        = buf;
        draw_pos[0] = align_text(position[0], line_width, alignment);
        while (*iter)
        {
            codepoint = (u32)*iter++;
            draw_glyph(font, codepoint, draw_pos, scale, color);
            draw_pos[0] += font->glyphs[codepoint].advance[0] * scale;
        }
        draw_pos[1] += row_size;
    }
}

static void
draw_glyph(const Font* font, u32 codepoint, vec2 position, float scale, vec4 color)
{
    vec2         draw_pos;
    vec2         size;
    float        u1, v1, u2, v2; /* texture coordinates                          */
    Vertex*      vert;           /* vertex mem ptr                               */
    const Glyph* glyph = &font->glyphs[codepoint];

    size[0] = glyph->size[0] * scale;
    size[1] = glyph->size[1] * scale;

    draw_pos[0] = position[0] + glyph->bearing[0] * scale;
    draw_pos[1] = position[1] - glyph->bearing[1] * scale;

    u1 = glyph->sprite.u1;
    v1 = glyph->sprite.v1;
    u2 = glyph->sprite.u2;
    v2 = glyph->sprite.v2;

    check_buffer_limit(1);

    vert = s_next_vert_ptr;

    // top-left corner
    vert->position[0]   = draw_pos[0];
    vert->position[1]   = draw_pos[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // top-right corner
    vert->position[0]   = draw_pos[0] + size[0];
    vert->position[1]   = draw_pos[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v1;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-right corner
    vert->position[0]   = draw_pos[0] + size[0];
    vert->position[1]   = draw_pos[1] + size[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u2;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);
    ++vert;

    // bottom-left corner
    vert->position[0]   = draw_pos[0];
    vert->position[1]   = draw_pos[1] + size[1];
    vert->position[2]   = 0;
    vert->tex_coords[0] = u1;
    vert->tex_coords[1] = v2;
    glm_vec4_copy(color, vert->color);
    ++vert;

    s_next_vert_ptr = vert;
    s_count++;
}

void
begin_text(const Shader* used_shader, mat4 used_mvp_matrix)
{
    ASSERT_MSG(!s_is_drawing, "already drawing");
    s_is_drawing    = true;
    s_next_vert_ptr = s_vert_buf;
    s_count         = 0;
    s_draw_call_cnt = 0;
    s_vert_cnt      = 0;
    s_used_shader   = used_shader ? used_shader : &s_dft_shader;
    glm_mat4_copy(used_mvp_matrix, s_used_mvp_matrix);
}

void
end_text()
{
    ASSERT_MSG(s_is_drawing, "call begin first!");
    submit();
    s_is_drawing = false;
}

void
text_renderer_query_statistics(RenderStatistics* statistics)
{
    statistics->draw_call_count = s_draw_call_cnt;
    statistics->vertex_count    = s_vert_cnt;
}

static void
submit(void)
{
    size_t vbo_updt_siz;
    size_t num_indices;

    if (s_count == 0)
        return;

    vbo_updt_siz = (sizeof(Vertex) * 4 * s_count);
    num_indices  = s_count * 6;

    /* update vertex buffer data */
    glBindBuffer(GL_ARRAY_BUFFER, s_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, (GLsizei)vbo_updt_siz, s_vert_buf);

    texture_bind(&s_used_font->atlas);
    shader_bind(s_used_shader);
    shader_upload_view_project_matrix(s_used_shader, s_used_mvp_matrix);

    /* draw all sprite in buffer */
    glBindVertexArray(s_vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)num_indices, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    /* reset state */
    s_count         = 0;
    s_next_vert_ptr = s_vert_buf;
    s_draw_call_cnt++;
    s_vert_cnt += num_indices;
}

static void
check_buffer_limit(u32 needed_space)
{
    if (needed_space + s_count > s_buffer_size)
    {
        submit();
    }
}
