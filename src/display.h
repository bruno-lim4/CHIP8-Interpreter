#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>

#define DISPLAY_WIDTH 64 // POWERS OF TWO
#define DISPLAY_HEIGHT 32

typedef struct display_ DISPLAY;

DISPLAY* createDisplay();
void updateDisplay(DISPLAY* display);
void cleanDisplay(DISPLAY* display);

void freeDisplay(DISPLAY* display);
void changePixelColor(DISPLAY* display, int x, int y, bool c);
bool getPixelColor(DISPLAY* display, int x, int y);

void printDisplay(DISPLAY* display);

#endif