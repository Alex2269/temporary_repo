// file draw_grid.h

#include "raylib.h"
#include "draw_grid.h"

// --- Прототипи функцій ---
void draw_grid(int screenWidth, int screenHeight);

void draw_grid_layer(int startX, int endX, int stepX,
                     int startY, int endY, int stepY,
                     Color color) {
  for (int y = startY; y <= endY; y += stepY)
    for (int x = startX; x <= endX; x += stepX)
      DrawRectangle(x, y, 2, 2, color);
}

void draw_grid(int screenWidth, int screenHeight) {
  int CELL_SIZE = 50;
  int DOT_SPACE = 5;

  // Розрахунок кількості клітинок по X і Y
  int cellsX = screenWidth / CELL_SIZE - 1;
  int cellsY = screenHeight / CELL_SIZE - 1;

  // Центрування сітки
  int offsetX = (screenWidth - cellsX * CELL_SIZE) / 2;
  int offsetY = (screenHeight - cellsY * CELL_SIZE) / 2;

  int startX = offsetX;
  int endX   = offsetX + cellsX * CELL_SIZE;
  int startY = offsetY;
  int endY   = offsetY + cellsY * CELL_SIZE;

  // Горизонтальні лінії (жовті точки)
  draw_grid_layer(startX, endX, CELL_SIZE / DOT_SPACE,
                  startY, endY, CELL_SIZE,
                  GRAY);

  // Вертикальні лінії (блакитні точки)
  draw_grid_layer(startX, endX, CELL_SIZE,
                  startY, endY, CELL_SIZE / DOT_SPACE,
                  SKYBLUE);
}

