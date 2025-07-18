#ifndef DISPLAY_H
#define DISPLAY_H

typedef struct display_ DISPLAY;

void cleanDisplay(DISPLAY* display);
DISPLAY* createDisplay();
void freeDisplay(DISPLAY* display);

#endif