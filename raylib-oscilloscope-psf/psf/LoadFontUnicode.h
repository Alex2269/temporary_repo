// LoadFontUnicode.h
#ifndef LOADFONTUNICODE_H
#define LOADFONTUNICODE_H

// Функція завантажує шрифт з підтримкою кирилиці та базових ASCII символів
// fnt - шлях до файлу шрифту (TTF, OTF тощо)
// fontSize - розмір шрифту у пікселях
// LineSpacing - відстань між рядками тексту при багаторядковому відображенні
Font LoadFontUnicode(char * fnt, int fontSize, int LineSpacing)
{
    // Загальна кількість символів для завантаження у атлас шрифту
    int codepointCount = 512;
    int codepoints[512] = { 0 };

    // Заповнюємо перші 95 позицій кодами базових ASCII символів (від 32 до 126)
    for (int i = 0; i < 95; i++) codepoints[i] = 32 + i;

    // Заповнюємо позиції з 96 по 350 кодами кириличних символів,
    // починаючи з Unicode-коду 928 (0x0400) — базовий блок кирилиці
    for (int i = 96; i < 351; i++) codepoints[i] = 928 + i;

    // Завантажуємо шрифт з розміром fontSize,
    // передаючи список кодів символів, які потрібно включити у текстуру
    Font font = LoadFontEx(fnt, fontSize, codepoints, codepointCount);

    // Встановлюємо білінійну фільтрацію для текстури шрифту,
    // що покращує якість масштабування і згладжування символів
    SetTextureFilter(font.texture, TEXTURE_FILTER_BILINEAR);

    // Встановлюємо відстань між рядками тексту для багаторядкового відображення
    SetTextLineSpacing(LineSpacing);

    return font;
}

#endif /* LOADFONTUNICODE_H */

