#ifndef GLYPHS_H
#define GLYPHS_H

#include "glyphmap.h"
#include "raylib.h"
#include <stdint.h>

extern void DrawPixel(int posX, int posY, Color color);

// Структура для опису шрифту повністю
typedef struct {
    const char* name;                  // Ім'я шрифту
    int glyph_width;                   // Ширина символу в пікселях
    int glyph_height;                  // Висота символу в пікселях
    int glyph_bytes;                   // Розмір гліфа (байт)
    const GlyphPointerMap* glyph_map;  // Масив гліфів Unicode -> glyph data
    int glyph_count;                   // Кількість гліфів у масиві
} RasterFont;

int utf8_strlen(const char* s);
int utf8_decode(const char* str, uint32_t* out_codepoint);

const GlyphPointerMap* FindGlyph(const RasterFont font, uint32_t unicode);

void DrawGlyph(const uint8_t* glyph, int charsize, int width, int height,
               uint16_t x, uint16_t y, Color color);

void DrawGlyphScaled(const uint8_t* glyph, int width, int height, int bytes_per_glyph,
                     int x, int y, int scale, Color color);

// Малювання одного символу (гліфа) за координатами (x,y)
void DrawChar(const RasterFont font, int x, int y, uint32_t codepoint, Color color);

// Малювання тексту з масштабуванням і відступами
void DrawTextScaled(const RasterFont font, int x, int y, const char* text,
                         int spacing, int scale, Color color);

// Малювання тексту з фоном та рамкою, кольори і відступи керуються параметрами
void DrawTextWithBackground(const RasterFont font, int x, int y, const char* text,
                            int spacing, int scale, Color textColor,
                            Color bgColor, Color borderColor,
                            int padding, int borderThickness);

// Малювання тексту з автоматичним інверсним фоном і рамкою
void DrawTextWithAutoInvertedBackground(const RasterFont font, int x, int y, const char* text,
                                        int spacing, int scale, Color textColor,
                                        int padding, int borderThickness);
#endif // GLYPHS_H

