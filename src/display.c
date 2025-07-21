#include "display.h"

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <SDL2/SDL.h>

#define SCALE 20

struct display_ {
    bool pixels[DISPLAY_HEIGHT][DISPLAY_WIDTH];
    SDL_Window* window;
    SDL_Renderer* renderer;
};  

// turn pixel at (x,y) on if c true. (update the renderer)
void changePixelColor(DISPLAY* display, int x, int y, bool c) {
    if (c) {
        SDL_SetRenderDrawColor(display->renderer, 255, 255, 255, 255);
        display->pixels[y][x] = 1;
    } else {
        SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
        display->pixels[y][x] = 0;
    }

    SDL_Rect rect = { x * SCALE, y * SCALE, SCALE, SCALE };
    SDL_RenderFillRect(display->renderer, &rect);
}

bool getPixelColor(DISPLAY* display, int x, int y) {
    return display->pixels[y][x];
}

void updateDisplay(DISPLAY* display) {
    SDL_RenderPresent(display->renderer);
}

// sets all to 0
void cleanDisplay(DISPLAY* display) {
    for(int i = 0; i < DISPLAY_HEIGHT; i++) {
        for(int j = 0; j < DISPLAY_WIDTH; j++) {
            display->pixels[i][j] = 0;
        }
    }
    SDL_SetRenderDrawColor(display->renderer, 0, 0, 0, 255);
    SDL_RenderClear(display->renderer);
}

DISPLAY* createDisplay() {
    DISPLAY* display = (DISPLAY *) malloc(sizeof(DISPLAY));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) < 0){
        printf("Couldn't initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }

    display->window = SDL_CreateWindow("CHIP8 - Interpreter", SDL_WINDOWPOS_UNDEFINED,
                        SDL_WINDOWPOS_UNDEFINED, SCALE * DISPLAY_WIDTH, SCALE * DISPLAY_HEIGHT, 0);
    if (!display->window){
        printf("Failed to open %d x %d window: %s\n", SCALE * DISPLAY_WIDTH, SCALE * DISPLAY_HEIGHT, SDL_GetError());
        exit(1);
    }

    display->renderer = SDL_CreateRenderer(display->window, -1, 0);
    
    cleanDisplay(display);

    return display;
}



void freeDisplay(DISPLAY* display) {
    SDL_DestroyWindow(display->window);
    SDL_DestroyRenderer(display->renderer);

    free(display);
    return;
}

/*
    DEBUG
*/

void printDisplay(DISPLAY* display) {
    for(int i = 0; i < DISPLAY_HEIGHT; i++) {
        for (int j = 0; j < DISPLAY_WIDTH; j++) {
            if (display->pixels[i][j])
                printf("*");
            else 
                printf("-");
        }
        printf("\n");
    }
}