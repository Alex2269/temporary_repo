#ifndef GLYPHMAP_H
#define GLYPHMAP_H

#include <stdint.h>

typedef struct {
    uint32_t unicode;
    const uint8_t* glyph;
} GlyphPointerMap;

#endif // GLYPHMAP_H
