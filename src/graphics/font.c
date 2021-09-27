#include "graphics/font.h"
#include "graphics/gl.h"

#include "ft2build.h"
#include FT_FREETYPE_H

#define FIRST_CODEPOINT 0x00
#define LAST_CODEPOINT 0xff

/* padding in case texture filter mode is linear */
#define PADDING 2

static FT_Library s_ft_library;

int
font_loader_init(void)
{
    int error;

    error = FT_Init_FreeType(&s_ft_library);
    if (error)
        return -1;
    return 0;
}

void
font_loader_shutdown(void)
{
    FT_Done_FreeType(s_ft_library);
    s_ft_library = NULL;
}

static void
convert_8bpp_to_32bpp(const u8* src, u8* dst, int length)
{
    int i = 0;
    while (i < length)
    {
        u8 c   = src[i];
        dst[0] = c;
        dst[1] = c;
        dst[2] = c;
        dst[3] = c;
        i++;
        dst += 4;
    }
}

int
font_load(Font* font, const char* file, u32 size)
{
    FT_Face      face = NULL;
    int          error;
    u32          glyph_index;
    int          status = 0;
    FT_GlyphSlot glyph;
    u32          max_glyph_width, max_glyph_height;
    int          x;
    GLuint       tex;
    const u32    num_glyphs = LAST_CODEPOINT - FIRST_CODEPOINT + 1;
    u32          atlas_width;
    u32          atlas_height;
    u8*          buff = NULL;

    error = FT_New_Face(s_ft_library, file, 0, &face);

    if (error)
    {
        UZU_ERROR("Failed to create font\n");
        return -1;
    }

    error = FT_Set_Pixel_Sizes(face, size, size);
    if (error)
    {
        UZU_ERROR("Failed to set font face size\n");
        status = -1;
        goto cleanup;
    }

    glyph            = face->glyph;
    max_glyph_width  = 0;
    max_glyph_height = 0;
    for (u32 codepoint = FIRST_CODEPOINT; codepoint < LAST_CODEPOINT; ++codepoint)
    {
        glyph_index = FT_Get_Char_Index(face, codepoint);

        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER) != 0)
        {
            printf("Failed to load character \'%x\'\n", codepoint);
            continue;
        }
        max_glyph_width  = maxu(glyph->bitmap.width, max_glyph_width);
        max_glyph_height = maxu(glyph->bitmap.rows, max_glyph_height);
    }

    atlas_width  = (max_glyph_width + PADDING) * num_glyphs;
    atlas_height = max_glyph_height;

    buff = SDL_malloc((sizeof *buff) * max_glyph_width * max_glyph_height * 4);

    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RGBA,
                 (int)atlas_width,
                 (int)atlas_height,
                 0,
                 GL_RGBA,
                 GL_UNSIGNED_BYTE,
                 0);

    x                  = 0;
    font->atlas.width  = (int)atlas_width;
    font->atlas.height = (int)atlas_height;
    font->atlas.handle = tex;
    font->glyph_cnt    = num_glyphs;
    font->glyphs       = SDL_malloc(sizeof(*font->glyphs) * font->glyph_cnt);
    int glyph_width, glyph_height;
    for (u32 i = FIRST_CODEPOINT; i < LAST_CODEPOINT; i++)
    {
        glyph_index = FT_Get_Char_Index(face, i);
        if (FT_Load_Glyph(face, glyph_index, FT_LOAD_RENDER))
            continue;

        glyph_width  = (int)glyph->bitmap.width;
        glyph_height = (int)glyph->bitmap.rows;

        convert_8bpp_to_32bpp(glyph->bitmap.buffer, buff, glyph_width * glyph_height);

        glTexSubImage2D(GL_TEXTURE_2D, 0, x, 0, glyph_width, glyph_height, GL_RGBA, GL_UNSIGNED_BYTE, buff);

        sprite_set_texture(&font->glyphs[i].sprite,
                           &font->atlas,
                           &(IntRect){ x, 0, glyph_width, glyph_height });
        font->glyphs[i].advance[0] = (float)(glyph->metrics.horiAdvance >> 6);
        font->glyphs[i].advance[1] = (float)(glyph->metrics.vertAdvance >> 6);
        font->glyphs[i].bearing[0] = (float)(glyph->bitmap_left);
        font->glyphs[i].bearing[1] = (float)(glyph->bitmap_top);
        font->glyphs[i].size[0]    = (float)glyph_width;
        font->glyphs[i].size[1]    = (float)glyph_height;
        font->max_glyph_width      = (float)max_glyph_width;
        font->max_glyph_height     = (float)max_glyph_height;

        x += (max_glyph_width + PADDING);
    }
cleanup:
    FT_Done_Face(face);
    SDL_free(buff);
    return status;
}

void
font_destroy(Font* font)
{
    SDL_free(font->glyphs);
    texture_destroy(&font->atlas);
}
