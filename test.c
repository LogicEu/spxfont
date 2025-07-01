#define SPXF_APPLICATION
#include <spxfont.h>
#include <stdio.h>
#include <stdlib.h>

static int spxTextureSave(const Tex2D tex, const char* path)
{
    int x, y, i = tex.width * tex.height;
    FILE* file = fopen(path, "w");
    if (!file) {
        fprintf(stderr, "could not write file '%s'\n", path);
        return EXIT_FAILURE;
    }

    fprintf(file, "P6 %d %d 255\n", tex.width, tex.height);
    for (y = tex.height - 1; y >= 0; --y) {
        i -= tex.width;
        for (x = 0; x < tex.width; ++x) {
            fwrite(tex.pixbuf + i + x, 1, 3, file);
        }
    }

    fprintf(stdout, "saved image file '%s'\n", path);
    return fclose(file);
}

int main(const int argc, const char** argv)
{
    static const char* hellostr = "Hello, World!";

    int ret;
    Font2D font;
    ivec2 p = {40, 40};
    Px col = {0, 255, 0, 255};
    Tex2D tex ={NULL, 150, 100};
    tex.pixbuf = calloc(tex.width * tex.height, sizeof(Px));

    /* font = spxFontLoad("/System/Library/Fonts/Supplemental/Arial.ttf"); */
    font = spxFontDefault;
    spxFontDrawText(tex, &font, argc > 1 ? argv[1] : hellostr, p, col);
    ret = spxTextureSave(tex, "out.ppm");

    /* spxFontFree(&font); */
    free(tex.pixbuf);
    return ret;
}

