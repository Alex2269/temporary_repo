// file draw_grid.h

#include "raylib.h"
#include "draw_grid.h"

void draw_grid_layer(int startX, int endX, int stepX,
                     int startY, int endY, int stepY,
                     Color color) {
  for (int y = startY; y <= endY; y += stepY)
    for (int x = startX; x <= endX; x += stepX)
      DrawRectangle(x, y, 2, 2, color);
}

void draw_grid(int screenWidth, int screenHeight, int cellSize, int padding) {
  // Визначаємо область для малювання сітки з урахуванням padding
  int startX = padding;
  int startY = padding;
  int endX = screenWidth - padding;
  int endY = screenHeight - padding;

  int width = endX - startX;
  int height = endY - startY;

  // Визначаємо скільки клітинок поміщається по горизонталі і вертикалі
  int cellsX = width / cellSize;
  int cellsY = height / cellSize;

  // Вираховуємо відступи, щоб сітка була центрована всередині області (без padding)
  int offsetX = startX + (width - cellsX * cellSize) / 2;
  int offsetY = startY + (height - cellsY * cellSize) / 2;

  // Малюємо точки сітки — шар з синіми горизонтальними і блакитними вертикальними
  int dotSpacing = cellSize / 5;

  // Горизонтальні лінії (сині)
  draw_grid_layer(offsetX, offsetX + cellsX * cellSize, dotSpacing,
                  offsetY, offsetY + cellsY * cellSize, cellSize,
                  BLUE);

  // Вертикальні лінії (блакитні)
  draw_grid_layer(offsetX, offsetX + cellsX * cellSize, cellSize,
                  offsetY, offsetY + cellsY * cellSize, dotSpacing,
                  SKYBLUE);
}
