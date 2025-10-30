// file draw_grid.h

#ifndef DRAW_GRID_H_
#define DRAW_GRID_H_

// --- Прототипи функцій ---
void draw_grid_layer(int startX, int endX, int stepX,
                     int startY, int endY, int stepY,
                     Color color);

void draw_grid(int screenWidth, int screenHeight, int cellSize, int padding);


#endif // DRAW_GRID_H_

