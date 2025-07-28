#ifndef CHIP8_H
#define CHIP8_H

#include <stdint.h>
#include <stdbool.h>

#include <SDL2/SDL.h>

typedef struct chip8_ CHIP8;

CHIP8* setupInterpreter(char* file_path);
void processNextInstruction(CHIP8* chip8);
void updateTimers(CHIP8* chip8);
void freeInterpreter(CHIP8* chip8);

void drawDisplay(CHIP8* chip8);
bool needsToDraw(CHIP8* chip8);
bool waitingForKey(CHIP8* chip8);
void handleKeyPressed(CHIP8* chip8, SDL_Event* event);

void printMemory(CHIP8* chip8);

#endif