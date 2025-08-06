// psf_font.c
// Підключення заголовочних файлів
#include "psf_font.h"       // Визначення структури шрифту та прототипів функцій
#include <stdio.h>          // Для роботи з файлами та виводу
#include <stdlib.h>         // Для динамічного виділення пам’яті
#include "UnicodeGlyphMap.h"// Відповідність Unicode кодів індексам гліфів

// Магічні числа для ідентифікації форматів PSF1 і PSF2
#define PSF1_MAGIC0 0x36
#define PSF1_MAGIC1 0x04

#define PSF2_MAGIC0 0x72
#define PSF2_MAGIC1 0xB5
#define PSF2_MAGIC2 0x4A
#define PSF2_MAGIC3 0x86

// Заголовок формату PSF1 (короткий)
typedef struct {
    unsigned char magic[2];  // Магічні байти для ідентифікації формату
    unsigned char mode;      // Режим (наприклад, кількість символів)
    unsigned char charsize;  // Розмір одного гліфа в байтах
} PSF1_Header;

// Заголовок формату PSF2 (більш розширений)
typedef struct {
    uint8_t magic[4];        // Магічні байти формату PSF2
    uint32_t version;        // Версія формату
    uint32_t headersize;     // Розмір заголовку
    uint32_t flags;          // Флаги (додаткова інформація)
    uint32_t length;         // Кількість символів (гліфів)
    uint32_t charsize;       // Розмір одного гліфа в байтах
    uint32_t height;         // Висота символу в пікселях
    uint32_t width;          // Ширина символу в пікселях
} PSF2_Header;

// Функція читання 4 байтів з файлу у форматі little-endian (молодший байт перший)
static uint32_t ReadLE32(FILE* f) {
    uint8_t b[4];
    fread(b, 1, 4, f);
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

// Функція декодування одного UTF-8 символу з рядка str
// Записує Unicode код символу у out_codepoint
// Повертає кількість байтів, які зайняв символ у UTF-8
static int utf8_decode(const char* str, uint32_t* out_codepoint) {
    unsigned char c = (unsigned char)str[0];
    if (c < 0x80) {
        // Однобайтовий ASCII символ
        *out_codepoint = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        // Два байти UTF-8
        *out_codepoint = ((str[0] & 0x1F) << 6) | (str[1] & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        // Три байти UTF-8
        *out_codepoint = ((str[0] & 0x0F) << 12) | ((str[1] & 0x3F) << 6) | (str[2] & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        // Чотири байти UTF-8
        *out_codepoint = ((str[0] & 0x07) << 18) | ((str[1] & 0x3F) << 12) | ((str[2] & 0x3F) << 6) | (str[3] & 0x3F);
        return 4;
    }
    // Невідомий або некоректний символ — повертаємо 0
    *out_codepoint = 0;
    return 1;
}

// Розмір таблиці відповідності Unicode → індекс гліфа
static int cyr_map_size = sizeof(cyr_map) / sizeof(cyr_map[0]);

// Функція пошуку індексу гліфа за Unicode кодом символу
static int UnicodeToGlyphIndex(uint32_t codepoint) {
    if (codepoint >= 32 && codepoint <= 126) {
        // Для ASCII символів індекс співпадає з кодом символу
        return (int)codepoint;
    }
    // Для кирилиці шукаємо у таблиці відповідності
    for (int i = 0; i < cyr_map_size; i++) {
        if (cyr_map[i].unicode == codepoint)
            return cyr_map[i].glyph_index;
    }
    // Якщо символ не знайдено, повертаємо індекс пробілу (32)
    return 32;
}

// Функція завантаження PSF шрифту з файлу filename
PSF_Font LoadPSFFont(const char* filename) {
    FILE* f = fopen(filename, "rb");
    if (!f) {
        printf("Не вдалося відкрити файл шрифту: %s\n", filename);
        exit(1);
    }

    unsigned char magic[4] = {0};
    fread(magic, 1, 4, f);  // Читаємо перші 4 байти для визначення формату

    PSF_Font font = {0};     // Ініціалізуємо структуру шрифту нулями

    if (magic[0] == PSF1_MAGIC0 && magic[1] == PSF1_MAGIC1) {
        // Якщо формат PSF1
        fseek(f, 0, SEEK_SET); // Повертаємось на початок файлу
        PSF1_Header header;
        fread(&header, sizeof(PSF1_Header), 1, f);

        font.isPSF2 = 0;
        font.width = 8;                   // Ширина символу в PSF1 завжди 8
        font.height = header.charsize;   // Висота символу = розмір гліфа
        font.charsize = header.charsize;
        font.charcount = (header.mode & 0x01) ? 512 : 256; // Кількість символів

        // Виділяємо пам’ять під гліфи та читаємо їх з файлу
        font.glyphBuffer = (unsigned char*)malloc(font.charcount * font.charsize);
        fread(font.glyphBuffer, font.charsize, font.charcount, f);
    }
    else if (magic[0] == PSF2_MAGIC0 && magic[1] == PSF2_MAGIC1 &&
             magic[2] == PSF2_MAGIC2 && magic[3] == PSF2_MAGIC3) {
        // Якщо формат PSF2
        PSF2_Header header;
        header.magic[0] = magic[0];
        header.magic[1] = magic[1];
        header.magic[2] = magic[2];
        header.magic[3] = magic[3];
        header.version = ReadLE32(f);
        header.headersize = ReadLE32(f);
        header.flags = ReadLE32(f);
        header.length = ReadLE32(f);
        header.charsize = ReadLE32(f);
        header.height = ReadLE32(f);
        header.width = ReadLE32(f);

        font.isPSF2 = 1;
        font.width = header.width;
        font.height = header.height;
        font.charcount = header.length;
        font.charsize = header.charsize;

        // Переходимо до початку гліфів (після заголовку)
        fseek(f, header.headersize, SEEK_SET);

        // Виділяємо пам’ять і читаємо гліфи
        font.glyphBuffer = (unsigned char*)malloc(font.charcount * font.charsize);
        fread(font.glyphBuffer, 1, font.charcount * font.charsize, f);
    }
    else {
        // Якщо формат не підтримується
        printf("Формат шрифту не підтримується або файл пошкоджено\n");
        fclose(f);
        exit(1);
    }

    fclose(f);
    return font;
}

// Функція звільнення пам’яті, виділеної під гліфи шрифту
void UnloadPSFFont(PSF_Font font) {
    free(font.glyphBuffer);
}

// Функція малювання одного символу (гліфа) у позиції (x,y) кольором color
void DrawPSFChar(PSF_Font font, int x, int y, int c, Color color) {
    if (c < 0 || c >= font.charcount) return; // Перевірка коректності індексу

    int width = font.width;
    int height = font.height;
    int bytes_per_row = (width + 7) / 8; // Кількість байтів на один рядок гліфа
    unsigned char* glyph = font.glyphBuffer + c * font.charsize; // Вказівник на гліф

    // Проходимо по кожному рядку гліфа
    for (int row = 0; row < height; row++) {
        // Проходимо по кожному байту в рядку
        for (int byte = 0; byte < bytes_per_row; byte++) {
            unsigned char bits = glyph[row * bytes_per_row + byte]; // Поточний байт
            // Перевіряємо кожен біт у байті
            for (int bit = 0; bit < 8; bit++) {
                int px = byte * 8 + bit; // Позиція пікселя по горизонталі
                if (px >= width) break;  // Якщо вийшли за ширину символу — зупиняємось
                if (bits & (0x80 >> bit)) {
                    // Якщо біт встановлений — малюємо піксель
                    DrawPixel(x + px, y + row, color);
                }
            }
        }
    }
}

// Функція малювання тексту UTF-8 шрифтом PSF з підтримкою переносу рядків '\n'
void DrawPSFText(PSF_Font font, int x, int y, const char* text, int spacing, Color color) {
    int xpos = x; // Поточна позиція по горизонталі
    int ypos = y; // Поточна позиція по вертикалі
    while (*text) {
        if (*text == '\n') {
            // Обробка переносу рядка:
            // повертаємося в початок по x та зсуваємо y вниз на висоту символу + відступ
            xpos = x;
            ypos += font.height + spacing;
            text++;
            continue;
        }
        uint32_t codepoint = 0;
        int bytes = utf8_decode(text, &codepoint); // Декодуємо один UTF-8 символ
        int glyph_index = UnicodeToGlyphIndex(codepoint); // Знаходимо індекс гліфа
        if (glyph_index < 0) glyph_index = 32; // Якщо символ не знайдено — замінюємо пробілом
        DrawPSFChar(font, xpos, ypos, glyph_index, color); // Малюємо символ
        xpos += font.width + spacing; // Зсуваємо позицію по x для наступного символу
        text += bytes; // Переходимо до наступного символу у тексті
    }
}

void DrawPSFCharScaled(PSF_Font font, int x, int y, int c, int scale, Color color) {
    if (c < 0 || c >= font.charcount) return;

    int width = font.width;
    int height = font.height;
    int bytes_per_row = (width + 7) / 8;
    unsigned char* glyph = font.glyphBuffer + c * font.charsize;

    for (int row = 0; row < height; row++) {
        for (int byte = 0; byte < bytes_per_row; byte++) {
            unsigned char bits = glyph[row * bytes_per_row + byte];
            for (int bit = 0; bit < 8; bit++) {
                int px = byte * 8 + bit;
                if (px >= width) break;
                if (bits & (0x80 >> bit)) {
                    // Малюємо квадрат розміром scale x scale пікселів
                    DrawRectangle(x + px * scale, y + row * scale, scale, scale, color);
                }
            }
        }
    }
}

void DrawPSFTextScaled(PSF_Font font, int x, int y, const char* text, int spacing, int scale, Color color) {
    int xpos = x;
    int ypos = y;
    while (*text) {
        if (*text == '\n') {
            xpos = x;
            ypos += (font.height * scale) + spacing;
            text++;
            continue;
        }
        uint32_t codepoint = 0;
        int bytes = utf8_decode(text, &codepoint);
        int glyph_index = UnicodeToGlyphIndex(codepoint);
        if (glyph_index < 0) glyph_index = 32;
        DrawPSFCharScaled(font, xpos, ypos, glyph_index, scale, color);
        xpos += (font.width * scale) + spacing;
        text += bytes;
    }
}

/* strlen рахує байти, а не символи UTF-8,
 * тому для кирилиці (2-3 байти на символ) ширина вважається завищеною.
 * Використання utf8_strlen поверне правильну кількість символів. */
int utf8_strlen(const char* s) {
    int len = 0;
    while (*s) {
        uint32_t codepoint = 0;
        int bytes = utf8_decode(s, &codepoint); // ваша функція декодування UTF-8
        s += bytes;
        len++;
    }
    return len;
}
