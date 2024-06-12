#ifndef TEXT_H
#define TEXT_H

#include "draw.h"
#include "etc.h"

#define DEF_CHAR_WIDTH 8
#define DEF_CHAR_HEIGHT 16

#define CHAR_WIDTH(chr, desc, width)                                                                   \
DO(                                                                                                    \
  if (!desc->width)                                                                                    \
    width = desc->maxwidth;                                                                            \
  else                                                                                                 \
    width = desc->width[chr - desc->firstchar];                                                        \
)                                                                                                      \

#define CHAR(x, y, scale_x, scale_y, chr, color)                                                       \
DO(                                                                                                    \
    font_descriptor_t *desc = &font_rom8x16;                                                           \
    uint8_t width;                                                                                     \
    CHAR_WIDTH(chr, desc, width);                                                                      \
    const font_bits_t *ptr;                                                                            \
    if ((chr >= desc->firstchar) && (chr - desc->firstchar < desc->size)){                             \
        if (desc->offset)                                                                              \
            ptr = &desc->bits[desc->offset[chr - desc->firstchar]];                                    \
        else{                                                                                          \
            int bw = (desc->maxwidth + 15) / 16;                                                       \
            ptr = &desc->bits[(chr - desc->firstchar) * bw * desc->height];                            \
        }                                                                                              \
                                                                                                       \
        for (int i = 0; i < desc->height; i++){                                                        \
            font_bits_t val = *ptr;                                                                    \
            for (int j = 0; j < width; j++){                                                           \
                if ((val & 0x8000) != 0)                                                               \
                    RECTANGLE(x + scale_x * j, y + scale_y * i, scale_x, scale_y, color);              \
                val <<= 1;                                                                             \
            }                                                                                          \
            ptr++;                                                                                     \
        }                                                                                              \
    }                                                                                                  \
)                                                                                                      \

#define TEXT(x, y, scale_x, scale_y, text, color)                                                      \
DO(                                                                                                    \
    uint16_t text_x = x - (strlen(text) * scale_x * DEF_CHAR_WIDTH / 2);                               \
    uint16_t text_y = y - scale_y * DEF_CHAR_HEIGHT / 2;                                               \
    for (int i = 0; i < strlen(text); i++){                                                            \
        CHAR(text_x, text_y, scale_x, scale_y, text[i], color);                                        \
        text_x += scale_x * DEF_CHAR_WIDTH;                                                            \
    }                                                                                                  \
)                                                                                                      \

#endif // TEXT_H