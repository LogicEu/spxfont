/*

Copyright (c) 2025 Eugenio Arteaga A.

Permission is hereby granted, free of charge, to any 
person obtaining a copy of this software and associated 
documentation files (the "Software"), to deal in the 
Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice 
shall be included in all copies or substantial portions
of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY
KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#ifndef SIMPLE_PIXEL_FONT_H
#define SIMPLE_PIXEL_FONT_H

/*********************
****** spxfont *******
   Simple PiXel Font
**********************
  @Eugenio Arteaga A.
**********************

Simple Pixel Font is a header only C library for
plotting fonts directly on images.

****************************************************/

/* Simple Pixel Font */

#include <stddef.h>
#include <stdint.h>

#ifndef PX_TYPE_DEFINED
#define PX_TYPE_DEFINED

typedef struct Px {
    uint8_t r, g, b, a;
} Px;

#endif /* PX_TYPE_DEFINED */

#ifndef TEX2D_TYPE_DEFINED
#define TEX2D_TYPE_DEFINED

typedef struct Tex2D {
    Px* pixbuf;
    int width;
    int height;
} Tex2D;

#endif /* TEX2D_TYPE_DEFINED */

#ifndef IVEC2_TYPE_DEFINED
#define IVEC2_TYPE_DEFINED

typedef struct ivec2 {
    int x, y;
} ivec2;

#endif /* IVEC2_TYPE_DEFINED */


typedef struct Glyph {
    unsigned char* pixmap;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
} Glyph;

typedef struct Font2D {
    struct FT_FaceRec_* face;
    Glyph* glyphs;
} Font2D;

Font2D  spxFontLoad(const char* path);
void    spxFontFree(Font2D* font);
void    spxFontSize(Font2D* font, size_t size);
int     spxFontDrawGlyph(Tex2D texture, const Glyph glyph, ivec2 p, const Px color);
int     spxFontDrawText(Tex2D texture, const Font2D*, const char*, ivec2, const Px);

#ifdef SPXF_APPLICATION

/*********************
****** spxfont *******
   Simple PiXel Font
**********************
*  IMPLEMENTATION    *
*********************/

#include <ft2build.h>
#include FT_FREETYPE_H

#ifndef SPXF_GLYPH_COUNT
#define SPXF_GLYPH_COUNT 128
#endif /* SPXF_GLYPH_COUNT */

#ifndef SPXF_FONT_SIZE 
#define SPXF_FONT_SIZE 12
#endif /* SPXF_FONT_SIZE */

static int ftInit = 0;
static FT_Library ft;

static uint8_t mix8(uint8_t a, uint8_t b, float t)
{
    return (uint8_t)((float)a + t * (float)(b - a));
}

static Px pxMix(Px a, Px b, float t)
{
    Px px;
    px.r = mix8(a.r, b.r, t);
    px.g = mix8(a.g, b.g, t);
    px.b = mix8(a.b, b.b, t);
    px.a = mix8(a.a, b.a, t);
    return px;
}

static int spxFontStart(void)
{
    if (FT_Init_FreeType(&ft)) {
        fprintf(stderr, "spxfont could not init freetype\n");
        return EXIT_SUCCESS;
    }

    ftInit = 1;
    return EXIT_FAILURE;
}

static void spxFontEnd(void)
{
    FT_Done_FreeType(ft);
    ftInit = 0;
}

void spxFontSize(Font2D* font, size_t size)
{
    size_t len;
    unsigned char c;
    FT_Set_Pixel_Sizes(font->face, 0, size);

    for (c = 0; c < SPXF_GLYPH_COUNT; ++c) {
        if (FT_Load_Char(font->face, c, FT_LOAD_RENDER)) {
            fprintf(stderr, "spxfont failed to load glyph: %c - $%d\n", c, (int)c);
            continue;
        }

        len = font->face->glyph->bitmap.width * font->face->glyph->bitmap.rows;
        font->glyphs[c].pixmap = (unsigned char*)realloc(font->glyphs[c].pixmap, len);
        font->glyphs[c].size.x = font->face->glyph->bitmap.width;
        font->glyphs[c].size.y = font->face->glyph->bitmap.rows;
        font->glyphs[c].bearing.x = font->face->glyph->bitmap_left;
        font->glyphs[c].bearing.y = font->face->glyph->bitmap_top;
        font->glyphs[c].advance = font->face->glyph->advance.x;
        memcpy(font->glyphs[c].pixmap, font->face->glyph->bitmap.buffer, len);
    }
}

void spxFontFree(Font2D* font)
{
    if (font->glyphs) {
        unsigned char c;
        for (c = 0; c < SPXF_GLYPH_COUNT; ++c) {
            if (font->glyphs[c].pixmap) {
                free(font->glyphs[c].pixmap);
                font->glyphs[c].pixmap = NULL;
            }
        }
        free(font->glyphs);
        font->glyphs = NULL;
    }

    FT_Done_Face(font->face);
    if (--ftInit == 1) {
        spxFontEnd();
    }
}

Font2D spxFontLoad(const char* path)
{ 
    Font2D font = {0};
    if (!ftInit) {
        spxFontStart();
    }

    if (FT_New_Face(ft, path, 0, &font.face)) {
        fprintf(stderr, "spxfont failed to load font: %s\n", path);
        return font;
    }

    font.glyphs = (Glyph*)calloc(sizeof(Glyph), SPXF_GLYPH_COUNT);
    spxFontSize(&font, SPXF_FONT_SIZE);
    ++ftInit;
    return font;
}

int spxFontDrawGlyph(Tex2D texture, const Glyph glyph, ivec2 p, const Px color)
{
    int x, y, ret = 0;
    const int resx = texture.width - 1, resy = texture.height - 1;
    const int endx = p.x + glyph.size.x, endy = p.y + glyph.size.y;
    ret = p.y < 0 || p.x < 0;
    for (y = p.y; y < endy; ++y) {
        ret += y > resy;
        for (x = p.x; x < endx; ++x) {
            Px* px = texture.pixbuf + y * texture.width + x;
            int index = (glyph.size.y - 1 - (y - p.y)) * glyph.size.x + (x - p.x);
            *px = pxMix(*px, color, (float)glyph.pixmap[index] / 255.0F);
            ret += x > resx;
        }
    }
    return ret;
}

int spxFontDrawText(  Tex2D texture,  const Font2D* font, const char* text, 
                        ivec2 p,        const Px color)
{
    int i, ret = 0;
    for (i = 0; text[i]; i++) {
        ret += spxFontDrawGlyph(texture, font->glyphs[(int)text[i]], p, color);
        p.x += (font->glyphs[(int)text[i]].advance >> 6);
        if (p.x >= texture.width) {
            ++ret;
            break;
        }
    }
    return ret;
}

#endif /* SPXF_APPLICATION */
#endif /* SIMPLE_PIXEL_FONT_H */

