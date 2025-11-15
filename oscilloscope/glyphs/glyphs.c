// glyphs.c

#include "glyphs.h"
// #include "graphics.h"
#include "raylib.h"
#include "color_utils.h"
#include <stdio.h>
#include <string.h>

/*
 * utf8_strlen - підрахунок кількості Unicode символів у UTF-8 рядку.
 * Стандартна strlen рахує байти, а не символи, що може спотворювати довжину тексту,
 * особливо для кирилиці та інших багатобайтових символів.
 */
int utf8_strlen(const char* s) {
    int len = 0;
    // Проходимо по рядку, декодуючи кожен UTF-8 символ
    while (*s) {
        uint32_t codepoint = 0;
        int bytes = utf8_decode(s, &codepoint); // розпаковуємо один символ
        s += bytes; // переходимо до наступного символу
        len++; // лічильник символів
    }
    return len;
}

/*
 * utf8_decode - декодування одного UTF-8 символу з початку рядка str.
 * Записує decodед Unicode код символу у out_codepoint.
 * Повертає довжину символу у байтах (1-4).
 * Якщо символ некоректний, повертає 1 і код 0.
 */
int utf8_decode(const char* str, uint32_t* out_codepoint) {
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80) {
        // Однобайтовий ASCII символ
        *out_codepoint = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        // Дво-байтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        // Трибайтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        // Чотири байтовий UTF-8 символ
        *out_codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) |
        ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        return 4;
    }
    // Некоректний символ
    *out_codepoint = 0;
    return 1;
}

/*
 * FindGlyph - пошук у шрифті font гліфа за Unicode кодом unicode.
 * Повертає вказівник на GlyphPointerMap, якщо знайдений, або NULL, якщо символ відсутній.
 * Лінійний пошук, достатній для невеликої кількості гліфів.
 */
const GlyphPointerMap* FindGlyph(const RasterFont font, uint32_t unicode) {
    for (int i = 0; i < font.glyph_count; i++) {
        if (font.glyph_map[i].unicode == unicode) {
            return &font.glyph_map[i];
        }
    }
    return NULL;
}

/*
 * DrawGlyph - малювання гліфа (bitmap) glyph в позиції (x,y) з кольором color.
 * charsize – розмір гліфа у байтах (не використовується напряму, але є для сумісності).
 * width, height – розміри гліфа у пікселях.
 * Малює всі встановлені біти як активні пікселі.
 */
void DrawGlyph(const uint8_t* glyph, int charsize, int width, int height,
               uint16_t x, uint16_t y, Color color) {
    int bytes_per_row = (width + 7) / 8; // кількість байтів для одного рядка

    for (int row = 0; row < height; row++) {
        for (int byte = 0; byte < bytes_per_row; byte++) {
            uint8_t bits = glyph[row * bytes_per_row + byte];
            for (int bit = 0; bit < 8; bit++) {
                int px = byte * 8 + bit; // позиція пікселя в рядку
                if (px >= width) break; // не виходимо за межі ширини
                if (bits & (0x80 >> bit)) {
                    // малюємо піксель, якщо біт встановлений
                    DrawPixel(x + px, y + row, color);
                }
            }
        }
    }
}

/*
 * DrawGlyphScaled - малювання гліфа з масштабуванням scale.
 * Для кожного активного пікселя малюється квадрат розміром scale x scale.
 */
void DrawGlyphScaled(const uint8_t* glyph, int width, int height, int bytes_per_glyph,
                     int x, int y, int scale, Color color) {
    int bytes_per_row = (width + 7) / 8;
    for (int row = 0; row < height; row++) {
        for (int byte = 0; byte < bytes_per_row; byte++) {
            uint8_t bits = glyph[row * bytes_per_row + byte];
            for (int bit = 0; bit < 8; bit++) {
                int px = byte * 8 + bit; // горизонтальне положення пікселя
                if (px >= width) break;
                if (bits & (0x80 >> bit)) {
                    int draw_x = x + px * scale;
                    int draw_y = y + row * scale;
                    // Малюємо квадрат із розміром scale
                    for (int dx = 0; dx < scale; dx++) {
                        for (int dy = 0; dy < scale; dy++) {
                            DrawPixel(draw_x + dx, draw_y + dy, color);
                        }
                    }
                }
            }
        }
    }
}

/*
 * DrawChar - малює один символ codepoint в позиції (x,y) з кольором color,
 * використовуючи гліфи шрифту font.
 * Якщо гліф не знайдено - нічого не малює.
 */
void DrawChar(const RasterFont font, int x, int y, uint32_t codepoint, Color color) {
    const GlyphPointerMap* glyph = FindGlyph(font, codepoint);
    if (!glyph) return; // якщо гліф не знайдено, вихід
    DrawGlyph(glyph->glyph, font.glyph_bytes, font.glyph_width, font.glyph_height, x, y, color);
}

/*
 * DrawTextScaled - малює текст text, починаючи з позиції (x,y),
 * використовуючи шрифт font зі масштабом scale, відступом spacing між символами
 * та кольором color. Функція підтримує перенос рядка \n.
 */
void DrawTextScaled(const RasterFont font, int x, int y, const char* text,
                         int spacing, int scale, Color color) {
    int xpos = x; // поточна позиція по горизонталі
    int ypos = y; // поточна позиція по вертикалі
    while (*text) {
        if (*text == '\n') {
            xpos = x; // перенос на початок рядка
            ypos += (font.glyph_height * scale) + spacing; // рух вниз з урахуванням масштабу і відступу
            text++;
            continue;
        }
        uint32_t codepoint = 0;
        int bytes = utf8_decode(text, &codepoint); // декодуємо один символ
        const GlyphPointerMap* glyph = FindGlyph(font, codepoint); // шукаємо гліф
        if (!glyph) glyph = FindGlyph(font, 32); // якщо не знайдено - використовуємо пробіл
        if (glyph) {
            DrawGlyphScaled(glyph->glyph, font.glyph_width, font.glyph_height, font.glyph_bytes,
                            xpos, ypos, scale, color);
        }
        // Рух праворуч з урахуванням ширини символу, масштабу та відступу
        xpos += (font.glyph_width * scale) + spacing;
        text += bytes; // переходимо до наступного символу
    }
}

/*
 * DrawTextWithBackground - малює текст із фоновим прямокутником та рамкою.
 * Параметри:
 * - font: шрифт для малювання
 * - x, y: позиція початку тексту
 * - text: текст для малювання (підтримує \n)
 * - spacing: відступ між символами
 * - scale: масштаб символів
 * - textColor, bgColor, borderColor: кольори тексту, фону і рамки
 * - padding: внутрішній відступ між текстом і рамкою
 * - borderThickness: товщина рамки (число проходів малювання прямокутника)
 */
void DrawTextWithBackground(const RasterFont font, int x, int y, const char* text,
                            int spacing, int scale, Color textColor,
                            Color bgColor, Color borderColor,
                            int padding, int borderThickness) {
    // Розбиваємо текст на рядки для посторочного малювання
    const char* lines[20];
    int lineCount = 0;
    char tempText[512];
    strncpy(tempText, text, sizeof(tempText) - 1);
    tempText[sizeof(tempText) - 1] = '\0';

    char* line = strtok(tempText, "\n");
    while (line != NULL && lineCount < 20) {
        lines[lineCount++] = line;
        line = strtok(NULL, "\n");
    }

    // Визначаємо максимальну довжину серед рядків (кількість Unicode символів)
    int maxLineChars = 0;
    for (int i = 0; i < lineCount; i++) {
        int len = utf8_strlen(lines[i]);
        if (len > maxLineChars) maxLineChars = len;
    }

    // Обчислюємо ширину та висоту фонового прямокутника з урахуванням масштабу і паддінгів
    int bgWidth = maxLineChars * (font.glyph_width * scale + spacing) - spacing + 2 * padding + 2 * borderThickness;
    int bgHeight = lineCount * (font.glyph_height * scale) + (lineCount - 1) * spacing + 2 * padding + 2 * borderThickness;

    // Малюємо залитий фон
    DrawRectangle(x - padding - borderThickness, y - padding - borderThickness, bgWidth, bgHeight, bgColor);

    // Малюємо рамку товщиною borderThickness
    for(int i = 0; i < borderThickness; i++) {
        DrawRectangleLines(x - padding - borderThickness + i, y - padding - borderThickness + i,
                 bgWidth - 2*i, bgHeight - 2*i, borderColor);
    }

    // Малюємо текст по рядках поверх фону і рамки
    int ypos = y;
    for (int i = 0; i < lineCount; i++) {
        DrawTextScaled(font, x, ypos, lines[i], spacing, scale, textColor);
        ypos += font.glyph_height * scale + spacing;
    }
}

/*
 * DrawTextWithAutoInvertedBackground - малює текст з фоном,
 * колір якого обирається автоматично як контрастна інверсія кольору тексту.
 * Використовує DrawTextWithBackground, але фон вибирається автоматично.
 */
void DrawTextWithAutoInvertedBackground(const RasterFont font, int x, int y, const char* text,
                                        int spacing, int scale, Color textColor,
                                        int padding, int borderThickness) {
    // Підбираємо фон як інверсний і контрастний до textColor
    Color bgColor = GetContrastInvertColor(textColor);
    // Викликаємо основну функцію з автоматичним фоном і рамкою кольору textColor
    DrawTextWithBackground(font, x, y, text, spacing, scale, textColor, bgColor, textColor, padding, borderThickness);
}
