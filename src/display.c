#include "display.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define DISPLAY_WIDTH 64
#define DISPLAY_HEIGHT 32

struct display_ {
    bool pixels[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    SDL_Window* window;
    SDL_Renderer* renderer;
};  

// change the pixels array
void drawToTheDisplay(DISPLAY* display, int n, int x, int y) {

}

void showDisplay(DISPLAY* display) {
    // update the renderer using the pixels matrix, and present it

    SDL_RenderPresent(display->renderer);
}

DISPLAY* createDisplay() {
    DISPLAY* display = (DISPLAY *) malloc(sizeof(DISPLAY));
    cleanDisplay(display);

    if (SDL_Init(SDL_INIT_VIDEO) < 0){
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    display->window = SDL_CreateWindow("Example: 0", SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED, DISPLAY_WIDTH, DISPLAY_HEIGHT, 0);
    if (!display->window){
        printf("Failed to open %d x %d window: %s\n", DISPLAY_WIDTH, DISPLAY_HEIGHT, SDL_GetError());
        exit(1);
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
    SDL_RenderClear(display->renderer);

    return display;
}

void cleanDisplay(DISPLAY* display) {
    for(int i = 0; i < DISPLAY_HEIGHT; i++) {
        for(int j = 0; j < DISPLAY_WIDTH; j++) {
            display->pixels[i][j] = 0;
        }
    }
}

void freeDisplay(DISPLAY* display) {
    SDL_DestroyWindow(display->window);
    SDL_DestroyRenderer(display->renderer);
    SDL_Quit();

    free(display);
    return;
}